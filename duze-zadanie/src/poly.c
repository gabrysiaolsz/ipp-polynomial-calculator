#include "poly.h"
#include "safe_memory_allocation.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};

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

bool RecursiveIsCoeff(const Poly *p) {
    if (PolyIsCoeff(p)) {
        return true;
    }
    for (size_t i = 0; i < p->size; ++i) {
        assert(i == 0 || MonoGetExp(&p->arr[i]) > MonoGetExp(&p->arr[i - 1]));
    }
    if (p->size == 0) {
        return true;
    }
    if (p->arr[0].exp != 0) {
        return false;
    }
    for (size_t i = 1; i < p->size; i++) {
        if (!RecursiveMonoIsZero(&p->arr[i])) {
            return false;
        }
    }

    return RecursiveIsCoeff(&p->arr[0].p);
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

    if (RecursiveIsCoeff(&newPoly)) {
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

    Mono *monos = SafeMalloc(size * sizeof(Mono));

    monos[0] = MonoFromPoly(p, 0);

    for (size_t j = 1; j < size; j++) {
        monos[j] = MonoClone(&q->arr[j - 1]);
    }

    Poly newPoly = PolyAddMonos(size, monos);

    free(monos);

    return newPoly;
}

/**
 * Dodaje dwa wielomiany do siebie: obydwa z nich nie są wielomianami stałymi.
 * @param *p : wskaźnik na pierwszy wielomian,
 * @param *q : wskaźnik na drugi wielomian,
 * @return wielomian będący sumą powyższych wielomianów.
 */
Poly AddNonCoeffPolys(const Poly *p, const Poly *q) {
    size_t size = p->size + q->size;

    Mono *monos = SafeMalloc(size * sizeof(Mono));

    for (size_t i = 0; i < p->size; i++) {
        monos[i] = MonoClone(&p->arr[i]);
    }
    for (size_t j = 0; j < q->size; j++) {
        monos[p->size + j] = MonoClone(&q->arr[j]);
    }

    Poly newPoly = PolyAddMonos(size, monos);

    free(monos);

    return newPoly;
}

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
Poly PolyAdd(const Poly *p, const Poly *q) {
    Poly newPoly;
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { // Obydwa są wielomianami stałymi.
        return PolyFromCoeff(p->coeff + q->coeff);
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) { // Obydwa są wielomianami niestałymi.
        newPoly = AddNonCoeffPolys(p, q);
    } else { // Jeden jest wielomianem stałym, a drugi niestałym.
        if (PolyIsCoeff(p)) {
            newPoly = AddCoeffToPoly(p, q);
        } else {
            newPoly = AddCoeffToPoly(q, p);
        }
    }
    return newPoly;
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
Mono MonoNeg(const Mono *m) {
    return (Mono){.p = PolyNeg(&m->p), .exp = m->exp};
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian @f$p@f$
 * @return @f$-p@f$
 */
Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p))
        return (Poly){.arr = NULL, .coeff = (-1) * p->coeff};

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};

    for (size_t i = 0; i < p->size; i++) {
        polyCopy.arr[i] = MonoNeg(&p->arr[i]);
    }

    return polyCopy;
}

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p - q@f$
 */
Poly PolySub(const Poly *p, const Poly *q){

    Poly minusQ = PolyNeg(q);
    Poly resultPoly = PolyAdd(p, &minusQ);
    PolyDestroy(&minusQ);

    return resultPoly;
}

poly_exp_t RecursivePolyDeg(const Poly *p, poly_exp_t tmp, poly_exp_t *max){
    if(PolyIsZero(p)){
        return -1;
    }
    if(PolyIsCoeff(p)){
        return 0;
    }

    for(size_t i = 0; i < p->size; i++){
        tmp += p->arr[i].exp;
        if(tmp > *max){
            *max = tmp;
        }
        RecursivePolyDeg(&p->arr[i].p, tmp, max);
    }
    return tmp;
}

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p){
    poly_exp_t max = -1;
    RecursivePolyDeg(p, 0, &max);

    return max - 1;
}