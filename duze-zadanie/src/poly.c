#include "poly.h"
#include "safe_memory_allocation.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// TODO camel case

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian do usunięcia.
 */
void PolyDestroy(Poly *p) {
    if (PolyIsCoeff(p)) {
        return;
    }

    for (size_t i = 0; i < p->size; i++) {
        MonoDestroy(&p->arr[i]);
    }
    free(p->arr);
}

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian do skopiowania,
 * @return skopiowany wielomian.
 */
Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p))
        return (Poly){.arr = NULL, .coeff = p->coeff};

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(polyCopy.size * sizeof(Mono))};

    for (size_t i = 0; i < p->size; i++) {
        polyCopy.arr[i] = MonoClone(&p->arr[i]);
    }

    return polyCopy;
}
/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] p : jednomian @f$m@f$,
 * @param[in] q : jednomian @f$n@f$.
 * @return @f$m = n@f$.
 */
bool MonoIsEq(const Mono *m, const Mono *n) {
    if (m->exp != n->exp) {
        return false;
    }

    return PolyIsEq((const Poly *)&m->p, (const Poly *)&n->p);
}

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian @f$p@f$,
 * @param[in] q : wielomian @f$q@f$.
 * @return @f$p = q@f$.
 */
bool PolyIsEq(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return (p->coeff == q->coeff);
    }
    if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        if (p->size == q->size) {
            for (size_t i = 0; i < p->size; i++) {
                if (!MonoIsEq(&p->arr[i], &q->arr[i])) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

/**
 * Porównuje wykładniki jednomianów.
 * @param p1 : wskaźnik na pierwszy jednomian do porównania,
 * @param p2 : wskaźnik na drugi jednomian do porównania.
 * @return : liczba ujemna gdy wykładnik pierwszego jednomianu jest mniejszy od drugiego,
 * 0 gdy wykładniki są równe, liczba dodatnia gdy wykładnik pierwszego wielomianu jest większy.
 */
int CompareMonosByExp(const void *p1, const void *p2) {
    Mono mono1 = *(Mono *)p1, mono2 = *(Mono *)p2;

    return mono1.exp - mono2.exp;
}

/**
 * Sortuje tablicę jednomianów po wykładnikach.
 * @param count : rozmiar tablicy jednomianów,
 * @param monos : tablica jednomianów.
 */
void SortMonosByExp(size_t count, Mono *monos) {
    qsort(monos, count, sizeof(Mono), CompareMonosByExp);
}

/**
 * Płytko sprawdza, czy jednomian jest zerowy.
 * @param mono : jednomian do sprawdzenia,
 * @return true - jeśli jednomian jest zerowy, false w przeciwnym przypadku.
 */
bool MonoIsZero(Mono *mono) {
    return PolyIsZero(&mono->p);
}

/**
 * Rekurencyjnie i głęboko sprawdza czy jednomian jest zerowy.
 * @param mono : jednomian do sprawdzenia,
 * @return true - jeśli jednomian jest zerowy, false w przeciwnym przypadku.
 */
bool RecursiveMonoIsZero(Mono *mono) {
    if (PolyIsCoeff(&mono->p)) {
        return PolyIsZero(&mono->p);
    } else {
        for (size_t i = 0; i < mono->p.size; i++) {
            if (!RecursiveMonoIsZero(&mono->p.arr[i])) {
                return false;
            }
        }
        return true;
    }
}

Mono AddMonos(Mono *first, Mono *second);
// TODO rozmowa z peczarem o safemallocu

Poly AddMonosArray(size_t count, Mono *monos) {
    size_t polyI = 0, monosI = 0;
    Poly newPoly = {.size = count, .arr = SafeMalloc(count * sizeof(Mono))};

    while (monosI < count) {
        newPoly.arr[polyI] = MonoClone(&monos[monosI]);
        monosI++;

        while (monosI < count && monos[monosI].exp == monos[monosI - 1].exp) {
            Mono temp = newPoly.arr[polyI];
            newPoly.arr[polyI] = AddMonos(&temp, &monos[monosI]);
            MonoDestroy(&temp);

            if (RecursiveMonoIsZero(&newPoly.arr[polyI])) {
                poly_exp_t tempExp = newPoly.arr[polyI].exp;
                MonoDestroy(&newPoly.arr[polyI]);
                newPoly.arr[polyI] = (Mono){.exp = tempExp, .p = PolyZero()};
            }
            monosI++;
        }

        if (MonoIsZero(&newPoly.arr[polyI])) {
            MonoDestroy(&newPoly.arr[polyI]);
        } else {
            polyI++;
        }
    }
    newPoly.size = polyI;
    return newPoly;
}

/**
 * Sumuje tablicę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów,
 * @param[in] monos : tablica jednomianów,
 * @return wielomian będący sumą jednomianów.
 */
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    SortMonosByExp(count, (Mono *)monos);

    Poly newPoly = AddMonosArray(count, (Mono *)monos);

    if (newPoly.size == 0 || (newPoly.size == 1 && newPoly.arr[0].exp == 0)) {
        newPoly.coeff = newPoly.arr[0].p.coeff;
        free(newPoly.arr);
        newPoly.arr = NULL;
    }

    for (size_t j = 0; j < count; j++) { // Czyszczenie pamięci po tablicy monos.
        MonoDestroy((Mono *)&monos[j]);
    }

    return newPoly;
}

/**
 * Dodaje dwa wielomiany do siebie: jeden z nich jest współczynnikiem, drugi wielomianem o
 * niezerowym wykładniku.
 * @param *p : wskaźnik na wielomian stały - współczynnik,
 * @param *q : wskaźnik na wielomian o niezerowym wykładniku,
 * @return wielomian będący sumą powyższych wielomianów.
 */
Poly AddCoeffToPoly(const Poly *p, const Poly *q) {
    assert(PolyIsCoeff(p));

    if (PolyIsZero(p)) {
        return PolyClone(q);
    }

    size_t size = q->size + 1;
    Poly newPoly = {.size = size, .arr = SafeMalloc(size * sizeof(Mono))};

    newPoly.arr[0] = MonoFromPoly(p, 0);
    size_t newPolyIndex = 0;

    if (q->arr[0].exp == 0) { // Pierwszy element z wielomianu niestałego to c * x^0.
        newPoly.arr[0].p.coeff += q->arr[0].p.coeff;
        newPoly.size--;
        newPolyIndex++;
    }

    while (newPolyIndex < q->size) {
        newPoly.arr[newPolyIndex] = MonoClone(&q->arr[newPolyIndex]);
        newPolyIndex++;
    }

    return newPoly;
}

/**
 * Robi głęboką kopię jednomianów od danego indeksu, zapisując je w przekazanym w argumencie
 * wielomianie wynikowym,
 * @param *p : wskaźnik na wielomian z którego będziemy kopiować jednomiany,
 * @param *pIndex : wskaźnik na indeks od którego miejsca zacząć kopiowanie,
 * @param *newPoly : wskaźnik na wielomian wynikowy do którego przekopiujemy jednomiany,
 * @param *newPolyIndex : wskaźnik na indeks od którego zacząć zapisywanie jednomianów.
 * @return
 */
void CopyMonos(const Poly *p, size_t pIndex, Poly *newPoly, size_t *newPolyIndex) {
    while (pIndex < p->size) {
        newPoly->arr[*newPolyIndex] = MonoClone(&p->arr[pIndex]);
        pIndex++, (*newPolyIndex)++;
    }
}

/**
 * Dodaje dwa wielomiany do siebie: obydwa z nich nie są wielomianami stałymi.
 * @param *p : wskaźnik na pierwszy wielomian,
 * @param *q : wskaźnik na drugi wielomian,
 * @return wielomian będący sumą powyższych wielomianów.
 */
Poly AddNonCoeffPolys(const Poly *p, const Poly *q) {
    Poly newPoly = {.size = p->size + q->size,
                    .arr = SafeMalloc((p->size + q->size) * sizeof(Mono))};
    // TODO sprawdzic czy nie robie tak wiecej
    size_t pIndex = 0, qIndex = 0, newPolyIndex = 0;

    while (pIndex < p->size && qIndex < q->size) {
        if (p->arr[pIndex].exp == q->arr[qIndex].exp) {
            newPoly.arr[newPolyIndex] = AddMonos(&p->arr[pIndex], &q->arr[qIndex]);
            pIndex++, qIndex++;
        } else {
            if (p->arr[pIndex].exp < q->arr[qIndex].exp) {
                newPoly.arr[newPolyIndex] = MonoClone(&p->arr[pIndex]);
                pIndex++;
            } else {
                newPoly.arr[newPolyIndex] = MonoClone(&q->arr[qIndex]);
                qIndex++;
            }
        }
        newPolyIndex++;
    }

    if (pIndex < p->size) {
        CopyMonos(p, pIndex, &newPoly, &newPolyIndex);
    } else if (qIndex < q->size) {
        CopyMonos(q, qIndex, &newPoly, &newPolyIndex);
    }
    newPoly.size = newPolyIndex;

    return newPoly;
}

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { // Obydwa są wielomianami stałymi.
        return PolyFromCoeff(p->coeff + q->coeff);
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) { // Obydwa są wielomianami niestałymi.
        return AddNonCoeffPolys(p, q);
    } else { // Jeden jest wielomianem stałym, a drugi niestałym.
        if (PolyIsCoeff(p)) {
            return AddCoeffToPoly(p, q);
        } else {
            return AddCoeffToPoly(q, p);
        }
    }
}

/**
 * Sumuje dwa jednomiany o tym samym wykładniku.
 * @param[in] *first : pierwszy jednomian,
 * @param[in] *second : drugi jednomian,
 * @return jednomian będący sumą jednomianów.
 */
Mono AddMonos(Mono *first, Mono *second) {
    assert(first->exp == second->exp);

    return (Mono){.exp = first->exp, .p = PolyAdd(&first->p, &second->p)};
}

/**
 * Zwraca przeciwny jednomian.
 * @param[in] p : jednomian @f$m@f$
 * @return @f$-m@f$
 */
Mono MonoNeg(const Mono *m){
    return (Mono) {.p = PolyNeg(&m->p), .exp = m->exp};
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian @f$p@f$
 * @return @f$-p@f$
 */
Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p))
        return (Poly){.arr = NULL, .coeff = (-1) * p->coeff};

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(polyCopy.size * sizeof(Mono))};

    for (size_t i = 0; i < p->size; i++) {
        polyCopy.arr[i] = MonoNeg(&p->arr[i]);
    }

    return polyCopy;
}

// funkcja do debugowania
// TODO usunąć pod koniec
void print_poly(const Poly *p) {
    if (PolyIsCoeff(p)) {
        printf("coeff rowny %ld\n", p->coeff);
        return;
    }
    for (size_t i = 0; i < p->size; i++) {
        printf("exp: %d, ", p->arr[i].exp);
        print_poly(&(p->arr[i].p));
    }
}