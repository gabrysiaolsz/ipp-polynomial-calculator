/** @file
 * Interfejs wykonujący podstawowe działania na wielomianach rzadkich wielu
 * zmiennych.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 04.2021
 */

#include "poly.h"
#include "safe_memory_allocation.h"
#include <stdlib.h>
#include <stdio.h> //TODO usunac 

void PolyDestroy(Poly *p) {
    if (PolyIsCoeff(p)) {
        return;
    }

    for (size_t i = 0; i < p->size; i++) {
        MonoDestroy(&p->arr[i]);
    }
    free(p->arr);
}

Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p)) {
        return (Poly){.arr = NULL, .coeff = p->coeff};
    }

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};

    for (size_t i = 0; i < p->size; i++) {
        polyCopy.arr[i] = MonoClone(&p->arr[i]);
    }

    return polyCopy;
}

/**
 * Sprawdza równość dwóch jednomianów.
 * @param m : jednomian @f$m@f$,
 * @param n : jednomian @f$n@f$.
 * @return @f$m = n@f$.
 */
static inline bool MonoIsEq(const Mono *m, const Mono *n) {
    if (m->exp != n->exp) {
        return false;
    }

    return PolyIsEq(&m->p, &n->p);
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return (p->coeff == q->coeff);
    }
    if (!PolyIsCoeff(p) && !PolyIsCoeff(q) && p->size == q->size) {
        for (size_t i = 0; i < p->size; i++) {
            if (!MonoIsEq(&p->arr[i], &q->arr[i])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

/**
 * Porównuje wykładniki jednomianów.
 * @param m : jednomian @f$m@f$,
 * @param n : jednomian @f$n@f$,
 * @return : liczba ujemna gdy wykładnik pierwszego jednomianu jest mniejszy od
 * drugiego, 0 gdy wykładniki są równe, liczba dodatnia gdy wykładnik pierwszego
 * wielomianu jest większy.
 */
static inline int CompareMonosByExp(const void *m, const void *n) {
    const Mono mono1 = *(const Mono *)m, mono2 = *(const Mono *)n;

    return (mono1.exp > mono2.exp) - (mono1.exp < mono2.exp);
}

/**
 * Sortuje tablicę jednomianów po wykładnikach.
 * @param count : rozmiar tablicy jednomianów,
 * @param monos : tablica jednomianów.
 */
static inline void SortMonosByExp(size_t count, Mono *monos) {
    qsort(monos, count, sizeof(Mono), CompareMonosByExp);
}

/**
 * Płytko sprawdza, czy jednomian jest zerowy.
 * @param m : jednomian do sprawdzenia,
 * @return @f$m = 0@f$.
 */
static inline bool MonoIsZero(const Mono *m) {
    return PolyIsZero(&m->p);
}

bool RecursiveMonoIsZero(const Mono *m) {
    if (PolyIsCoeff(&m->p)) {
        return PolyIsZero(&m->p);
    }

    for (size_t i = 0; i < m->p.size; i++) {
        if (!RecursiveMonoIsZero(&m->p.arr[i])) {
            return false;
        }
    }
    return true;
}

bool RecursivePolyIsCoeff(const Poly *p, poly_coeff_t *result) {
    *result = 0;
    if (PolyIsCoeff(p)) {
        *result = p->coeff;
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

    return RecursivePolyIsCoeff(&p->arr[0].p, result);
}

/**
 * Sumuje dwa jednomiany o tym samym wykładniku.
 * @param m : jednomian @f$m@f$,
 * @param n : jednomian @f$m@f$,
 * @return @f$m + n@f$.
 */
static inline Mono AddMonos(const Mono *m, const Mono *n) {
    assert(m->exp == n->exp);

    return (Mono){.exp = m->exp, .p = PolyAdd(&m->p, &n->p)};
}

/**
 * Dodaje jednomiany z tablicy monos do siebie, tworząc nowy wielomian.
 * @param count : liczba jednomianów do dodania,
 * @param *monos : wskaźnik na tablicę jednomianów do dodania,
 * @return Poly : wielomian będący wynikiem dodawania jednomianów.
 */
static inline Poly AddMonosArray(size_t count, const Mono *monos) {
    size_t polyI = 0, monosI = 0;
    Poly newPoly = {.size = count, .arr = SafeMalloc(count * sizeof(Mono))};

    while (monosI < count) {
        newPoly.arr[polyI] = MonoClone(&monos[monosI++]);

        while (monosI < count && monos[monosI].exp == monos[monosI - 1].exp) {
            Mono temp = newPoly.arr[polyI];
            newPoly.arr[polyI] = AddMonos(&temp, &monos[monosI++]);
            MonoDestroy(&temp);

            if (RecursiveMonoIsZero(&newPoly.arr[polyI])) {
                poly_exp_t tempExp = newPoly.arr[polyI].exp;
                MonoDestroy(&newPoly.arr[polyI]);
                newPoly.arr[polyI] = (Mono){.exp = tempExp, .p = PolyZero()};
            }
        }

        if (!MonoIsZero(&newPoly.arr[polyI])) {
            polyI++;
        }
    }

    newPoly.size = polyI;
    if (newPoly.size == 0) {
        PolyDestroy(&newPoly);
        return PolyZero();
    }

    return newPoly;
}

Poly PolyOwnMonos(size_t count, Mono *monos){
    if(count == 0 || monos == NULL){
        return PolyZero();
    }

    SortMonosByExp(count, monos);
    Poly newPoly = AddMonosArray(count, monos);

    poly_coeff_t coeff;
    if (RecursivePolyIsCoeff((&newPoly), &coeff)) {
        poly_coeff_t tmp = coeff;
        PolyDestroy(&newPoly);
        newPoly.coeff = tmp;
        newPoly.arr = NULL;
    }

    for (size_t j = 0; j < count; j++) { // Czyszczenie pamięci po tablicy monos.
        MonoDestroy(&monos[j]);
    }
    free(monos);

    return newPoly;
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    Mono *newMonos = SafeMalloc(count * sizeof(Mono));
    for (size_t i = 0; i < count; i++) {
        newMonos[i] = monos[i];
    }

    return PolyOwnMonos(count, newMonos);
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    Mono *newMonos = SafeMalloc(count * sizeof(Mono));
    size_t size = 0;
    
    for (size_t i = 0; i < count; i++) {
        if(!MonoIsZero(&monos[i]))
        {
            newMonos[size] = monos[i];
            size++;
        }
    }
    
    return PolyOwnMonos(size, newMonos);
}

/**
 * Dodaje dwa wielomiany do siebie: jeden z nich jest współczynnikiem, drugi
 * wielomianem o niezerowym wykładniku.
 * @param p : wielomian @f$p@f$ stały - współczynnik,
 * @param q : wielomian @f$q@f$ o niezerowym wykładniku,
 * @return @f$p + q@f$ .
 */
static inline Poly AddCoeffToPoly(const Poly *p, const Poly *q) {
    assert(PolyIsCoeff(p));

    if (PolyIsZero(p)) {
        return PolyClone(q);
    }

    size_t size = q->size + 1;
    Mono *monos = SafeMalloc(size * sizeof(Mono));
    monos[0] = MonoFromPoly(p, 0);

    for (size_t i = 1; i < size; i++) {
        monos[i] = MonoClone(&q->arr[i - 1]);
    }

    Poly newPoly = PolyAddMonos(size, monos);

    free(monos);

    return newPoly;
}

/**
 * Dodaje dwa wielomiany do siebie: obydwa z nich nie są wielomianami stałymi.
 * @param p : wielomian @f$p@f$ o niezerowym wykładniku,
 * @param q : wielomian @f$q@f$ o niezerowym wykładniku,
 * @return @f$p + q@f$.
 */
static inline Poly AddNonCoeffPolys(const Poly *p, const Poly *q) {
    size_t size = p->size + q->size;

    Mono *monos = SafeMalloc(size * sizeof(Mono));

    for (size_t i = 0; i < p->size; i++) {
        monos[i] = MonoClone(&p->arr[i]);
    }
    for (size_t i = 0; i < q->size; i++) {
        monos[p->size + i] = MonoClone(&q->arr[i]);
    }

    Poly newPoly = PolyAddMonos(size, monos);

    free(monos);

    return newPoly;
}

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
 * Zwraca przeciwny jednomian.
 * @param m : jednomian @f$m@f$,
 * @return @f$-m@f$.
 */
static inline Mono MonoNeg(const Mono *m) {
    return (Mono){.p = PolyNeg(&m->p), .exp = m->exp};
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p)) {
        return (Poly){.arr = NULL, .coeff = -p->coeff};
    }

    Poly polyCopy = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};

    for (size_t i = 0; i < p->size; i++) {
        polyCopy.arr[i] = MonoNeg(&p->arr[i]);
    }

    return polyCopy;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly minusQ = PolyNeg(q);
    Poly resultPoly = PolyAdd(p, &minusQ);
    PolyDestroy(&minusQ);

    return resultPoly;
}

/**
 * Zwraca głęboki stopień jednomianu.
 * @param m : jednomian,
 * @return stopień jednomianu @p m.
 */
static inline poly_exp_t MonoDeg(const Mono *m) {
    if (PolyIsCoeff(&m->p)) {
        return MonoGetExp(m);
    }

    return MonoGetExp(m) + PolyDeg(&m->p);
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsZero(p)) {
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
 * @param m : jednomian @f$m@f$ ,
 * @param varIndex : indeks zmiennej,
 * @return stopień jednomianu @p m z względu na zmienną o indeksie @p varIndex.
 */
static inline poly_exp_t MonoDegBy(const Mono *m, size_t varIndex) {
    return PolyDegBy(&m->p, varIndex);
}

poly_exp_t PolyDegBy(const Poly *p, size_t varIndex) {
    if (PolyIsZero(p)) {
        return -1;
    }
    if (PolyIsCoeff(p)) {
        return 0;
    }
    if (varIndex == 0) {
        return MonoGetExp(&p->arr[p->size - 1]); // Zwracamy największą potęgę.
    }

    poly_exp_t max = 0;
    for (size_t i = 0; i < p->size; i++) {
        poly_exp_t curr = MonoDegBy(&p->arr[i], varIndex - 1);
        if (curr > max) {
            max = curr;
        }
    }
    return max;
}

/**
 * Mnoży dwa wielomiany jednomiany.
 * @param m : jednomian @f$m@f$,
 * @param n : jednomian @f$n@f$,
 * @return @f$m \cdot n@f$.
 */
static inline Mono MonoMul(const Mono *m, const Mono *n) {
    return (Mono){.exp = m->exp + n->exp, .p = PolyMul(&m->p, &n->p)};
}

/**
 * Mnoży dwa wielomiany stałe.
 * @param p : wielomian stały @f$p@f$,
 * @param q : wielomian stały @f$q@f$,
 * @return @f$p \cdot q@f$.
 */
static inline Poly MultiplyCoeffs(const Poly *p, const Poly *q) {
    assert(PolyIsCoeff(p) && PolyIsCoeff(q));

    return PolyFromCoeff(p->coeff * q->coeff);
}

/**
 * Mnoży dwa wielomiany niestałe - takie, które nie są współczynnikami.
 * @param p : wielomian o niezerowym wykładniku @f$p@f$,
 * @param q : wielomian o niezerowym wykładniku @f$q@f$,
 * @return @f$p \cdot q@f$.
 */
static inline Poly MultiplyNonCoeffs(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && !PolyIsCoeff(q));

    size_t count = p->size * q->size;
    Mono *monos = SafeMalloc(count * sizeof(Mono));

    for (size_t pIndex = 0, monosIndex = 0; pIndex < p->size; pIndex++) {
        for (size_t qIndex = 0; qIndex < q->size; qIndex++, monosIndex++) {
            monos[monosIndex] = MonoMul(&p->arr[pIndex], &q->arr[qIndex]);
        }
    }

    Poly result = PolyAddMonos(count, monos);
    free(monos);

    return result;
}

/**
 * Mnoży dwa wielomiany, pierwszym z nie jest wielomianem stałym, a drugi jest.
 * @param p : wielomian niebędący współczynnikiem @f$p@f$,
 * @param q : wielomian stały @f$q@f$,
 * @return @f$p \cdot q@f$.
 */
static inline Poly MultiplyPolyByCoeff(const Poly *p, const Poly *q) {
    assert(!PolyIsCoeff(p) && PolyIsCoeff(q));

    if (PolyIsZero(q)) {
        return PolyZero();
    }

    const Mono constValue = MonoFromPoly(q, 0);
    Poly newPoly = {.size = p->size, .arr = SafeMalloc(p->size * sizeof(Mono))};
    size_t newPolyI = 0;

    for (size_t i = 0; i < p->size; i++) {
        Mono tmp = MonoMul(&p->arr[i], &constValue);
        if (!MonoIsZero(&tmp)) {
            newPoly.arr[newPolyI++] = tmp;
        }
    }
    newPoly.size = newPolyI;

    if (newPoly.size == 0) {
        PolyDestroy(&newPoly);
        return PolyZero();
    }

    return newPoly;
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { // Obydwa są współczynnikami.
        return MultiplyCoeffs(p, q);
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) { // Obydwa nie są współczynnikami.
        return MultiplyNonCoeffs(p, q);
    } else if (!PolyIsCoeff(p) && PolyIsCoeff(q)) { // Jeden jest współczynnikiem, a drugi nie.
        return MultiplyPolyByCoeff(p, q);
    } else {
        return MultiplyPolyByCoeff(q, p);
    }
}

/**
 * Podnosi daną liczbę do potęgi wykorzystując szybkie potęgowanie binarne.
 * @param x : podstawa @f$x@f$,
 * @param n : wykładnik @f$n@f$,
 * @return @f$x ^ n@f$.
 */
static inline poly_coeff_t RaiseToPower(poly_coeff_t x, poly_exp_t n) {
    if (n == 0) {
        return 1;
    }

    poly_coeff_t res = 1;
    while (n > 0) {
        if (n & 1) {
            res *= x;
        }

        x *= x;
        n >>= 1;
    }

    return res;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
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

Poly PolyRaiseToPower(const Poly *p, poly_exp_t power){
    if(power == 0){
        return PolyFromCoeff(1);
    }
    
    if(PolyIsCoeff(p)){
        return PolyFromCoeff(RaiseToPower(p->coeff, power));
    }
    
    Poly polySquared = PolyRaiseToPower(p, power/2);
    Poly polyResult = PolyMul(&polySquared, &polySquared);
    
    if(power % 2 == 1){
        Poly polyTmp = polyResult;
        polyResult = PolyMul(p, &polyResult);
        PolyDestroy(&polyTmp);
    }

    PolyDestroy(&polySquared);
    
    return polyResult;
}

Poly PolyComposeByIndex(const Poly *p, size_t k, const Poly q[], size_t index);

Poly PolyComposeMono(const Mono *m, size_t k, const Poly q[], size_t index){
    Poly basePoly;
    if(index < k){
        basePoly = q[index];
    }
    else{
        basePoly = PolyZero();
    }
    
    Poly polyToMultiply = PolyRaiseToPower(&basePoly, m->exp);
    Poly nextPoly = PolyComposeByIndex(&m->p, k, q, index + 1);
    Poly resultPoly = PolyMul(&polyToMultiply, &nextPoly);

    PolyDestroy(&polyToMultiply);
    PolyDestroy(&nextPoly);
    
    return resultPoly;
}

Poly PolyComposeByIndex(const Poly *p, size_t k, const Poly q[], size_t index){
    if(PolyIsCoeff(p)){
        return PolyClone(p); //TODO trzeba?
    }
    
    Poly resultPoly = PolyZero();
    
    for(size_t i = 0; i < p->size; i++){
        Poly composePoly = PolyComposeMono(&p->arr[i], k, q, index);
        Poly tmpPoly = resultPoly;
        resultPoly = PolyAdd(&resultPoly, &composePoly);
        
        PolyDestroy(&tmpPoly);
        PolyDestroy(&composePoly);
    }
    
    return resultPoly;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]){
    return PolyComposeByIndex(p, k, q, 0);
}
