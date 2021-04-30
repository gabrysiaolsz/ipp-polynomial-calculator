#include "poly.h"
#include "safe_memory_allocation.h"
#include <stddef.h>
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

/**
 * Rekurencyjnie i głęboko sprawdza czy wielomian jest współczynnikiem.
 * @param p : wielomian do sprawdzenia,
 * @return true - jeśli wielomian jest współczynnikiem, false w przeciwnym przypadku.
 */
bool RecursivePolyIsCoeff(const Poly *p) {
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

    return RecursivePolyIsCoeff(&p->arr[0].p);
}

Mono AddMonos(Mono *first, Mono *second);


/**
 * Dodaje jednomiany z tablicy monos do siebie, tworząc nowy wielomian.
 * @param count : liczba jednomianów do dodania,
 * @param *monos : wskaźnik na tablicę jednomianów do dodania,
 * @return Poly : wielomian będący wynikiem dodawania jednomianów.
 */
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
 * Sumuje tablicę jednomianów i tworzy z nich wielomian. Sprawdza warunki brzegowe.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów,
 * @param[in] monos : tablica jednomianów,
 * @return wielomian będący sumą jednomianów.
 */
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    SortMonosByExp(count, (Mono *)monos);

    Poly newPoly = AddMonosArray(count, (Mono *)monos);

    if (RecursivePolyIsCoeff(&newPoly)) {
        poly_coeff_t tmp = newPoly.arr[0].p.coeff; // TODO pomyśleć o tym
        PolyDestroy(&newPoly);
        newPoly.coeff = tmp;
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
 * @param[in] p : wielomian @f$p@f$,
 * @param[in] q : wielomian @f$q@f$,
 * @return @f$p + q@f$.
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
 * @param[in] p : jednomian @f$m@f$,
 * @return @f$-m@f$.
 */
Mono MonoNeg(const Mono *m) {
    return (Mono){.p = PolyNeg(&m->p), .exp = m->exp};
}

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian @f$p@f$,
 * @return @f$-p@f$.
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
 * @param[in] p : wielomian @f$p@f$,
 * @param[in] q : wielomian @f$q@f$,
 * @return @f$p - q@f$.
 */
Poly PolySub(const Poly *p, const Poly *q) {
    Poly minusQ = PolyNeg(q);
    Poly resultPoly = PolyAdd(p, &minusQ);
    PolyDestroy(&minusQ);

    return resultPoly;
}

/**
 * Zwraca głęboki stopień jednomianu.
 * @param[in] m : jednomian,
 * @return stopień jednomianu @p m.
 */
poly_exp_t MonoDeg(const Mono *m) {
    if (PolyIsCoeff(&m->p)) {
        return MonoGetExp(m);
    }

    return MonoGetExp(m) * PolyDeg(&m->p);
}

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian,
 * @return stopień wielomianu @p p.
 */
poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsCoeff(p) && PolyIsZero(p)) {
        return -1;
    }
    if (PolyIsCoeff(p)) {
        return 0;
    }

    poly_exp_t max = 0;

    for (size_t i = 0; i < p->size; i++) {
        poly_exp_t curr = MonoDeg(&p->arr[i]);
        if (curr > max) {
            max = curr;
        }
    }

    return max;
}

/**
 * Zwraca stopień jednomianu ze względu na zadaną zmienną.
 * @param[in] m : jednomian,
 * @param[in] var_idx : indeks zmiennej,
 * @return stopień jednomianu @m p z względu na zmienną o indeksie @p var_idx.
 */
poly_exp_t MonoDegBy(const Mono *m, size_t var_idx) {
    return PolyDegBy(&m->p, var_idx - 1);
}

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru). Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian,
 * @param[in] var_idx : indeks zmiennej,
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx.
 */
poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    if (PolyIsCoeff(p) && PolyIsZero(p)) {
        return -1;
    }
    if (PolyIsCoeff(p)) {
        return 0;
    }

    if (var_idx == 0) {
        return MonoGetExp(&p->arr[p->size - 1]); // zwracamy największą potęgę
    }

    poly_exp_t max = 0;

    for (size_t i = 0; i < p->size; i++) {
        poly_exp_t curr = MonoDegBy(&p->arr[i], var_idx);

        if (curr > max) {
            max = curr;
        }
    }

    return max;
}

/**
 * Mnoży dwa jednomiany.
 * @param[in] m : jednomian @f$m@f$,
 * @param[in] n : jednomian @f$n@f$,
 * @return @f$m * n@f$.
 */
Mono MonoMul(const Mono *m, const Mono *n) {
    return (Mono){.exp = m->exp + n->exp, .p = PolyMul(&m->p, &n->p)};
}


/**
 * Mnoży dwa wielomiany stałe.
 * @param[in] p : wielomian stały @f$p@f$,
 * @param[in] q : wielomian stały @f$q@f$,
 * @return @f$p * q@f$.
 */
Poly MultiplyCoeffs(const Poly *p, const Poly *q) {
    assert(PolyIsCoeff(p) && PolyIsCoeff(q));
    return PolyFromCoeff(p->coeff * q->coeff);
}


/**
 * Mnoży dwa wielomiany niestałe - takie, które nie są współczynnikami.
 * @param[in] p : wielomian niebędący współczynnikiem @f$p@f$,
 * @param[in] q : wielomian niebędący współczynnikiem @f$q@f$,
 * @return @f$p * q@f$.
 */
Poly MultiplyNonCoeffs(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && !PolyIsCoeff(q));
    size_t count = p->size * q->size;
    Mono *monos = SafeMalloc(count * sizeof(Mono));
    size_t monosIndex = 0;

    for (size_t pIndex = 0; pIndex < p->size; pIndex++) {
        for (size_t qIndex = 0; qIndex < q->size; qIndex++) {
            monos[monosIndex] = MonoMul(&p->arr[pIndex], &q->arr[qIndex]);
            monosIndex++;
        }
    }

    Poly result = PolyAddMonos(count, monos);
    free(monos);

    return result;
}

/**
 * Mnoży dwa wielomiany, pierwszym z nie jest wielomianem stałym, a drugi jest.
 * @param[in] p : wielomian niebędący współczynnikiem @f$p@f$,
 * @param[in] q : wielomian stały @f$q@f$,
 * @return @f$p * q@f$.
 */
Poly MultiplyPolyByCoeff(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && PolyIsCoeff(q));
    if (PolyIsZero(q)) {
        return PolyZero();
    }
    Mono constValue = MonoFromPoly(q, 0);

    Poly newPoly = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};
    size_t newPolyI = 0;

    for (size_t polyI = 0; polyI < p->size; polyI++) {
        Mono tmp = MonoMul(&p->arr[polyI], &constValue);
        if (!MonoIsZero(&tmp)) { // overflow test od timy xd
            newPoly.arr[newPolyI] = tmp;
            newPolyI++;
        }
    }
    // tutaj jakieś reduce ??
    newPoly.size = newPolyI;

    return newPoly;
}

/**
 * Mnoży dwa dowolne wielomiany.
 * @param[in] p : wielomian @f$p@f$,
 * @param[in] q : wielomian @f$q@f$,
 * @return @f$p * q@f$.
 */
Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { // Obydwa są współczynnikami.
        return MultiplyCoeffs(p, q);
    }
    if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) { // Obydwa nie są współczynnikami.
        return MultiplyNonCoeffs(p, q);
    }
    if (!PolyIsCoeff(p) && PolyIsCoeff(q)) { //Jeden jest współczynnikiem, a drugi nie.
        return MultiplyPolyByCoeff(p, q);
    } else {
        return MultiplyPolyByCoeff(q, p);
    }
}

/**
 * Podnosi daną liczbę do potęgi.
 * @param[in] x : podstawa @f$x@f$,
 * @param[in] n : wykładnik @f$n@f$,
 * @return @f$x ^ n@f$.
 */
poly_coeff_t RaiseToPower(poly_coeff_t x, poly_exp_t n) {
    if (n == 0) {
        return 1;
    }

    poly_coeff_t tmp = RaiseToPower(x, n / 2);

    if (n % 2 == 0) {
        return tmp * tmp;
    } else {
        return x * tmp * tmp;
    }
}

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianami.
 * Wtedy zmniejszane są o jeden indeksy zmiennych w takim wielomianie.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] x : wartość argumentu @f$x@f$
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff * x);
    }

    Poly newPoly = PolyZero();

    for (size_t i = 0; i < p->size; i++) {
        Poly polyToPower = PolyFromCoeff(RaiseToPower(x, p->arr[i].exp));
        Poly multiplyResult = PolyMul(&p->arr[i].p, &polyToPower);
        PolyDestroy(&polyToPower);
        Poly tmp = newPoly;
        newPoly = PolyAdd(&newPoly, &multiplyResult);
        PolyDestroy(&multiplyResult);
        PolyDestroy(&tmp);
    }

    return newPoly;
}