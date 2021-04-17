#include "poly.h"
#include "safe_memory_allocation.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// TODO camel case

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
// TODO porozmawiac z tima czy ma tak samo
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
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p))
        return (Poly){.arr = NULL, .coeff = p->coeff};

    Poly poly_copy = {.size = p->size, .arr = safe_malloc(poly_copy.size * sizeof(Mono)) };

    for(size_t i = 0; i < p->size; i++){
        poly_copy.arr[i] = MonoClone(&p->arr[i]);
    }

    return poly_copy;
}
/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] p : jednomian @f$m@f$
 * @param[in] q : jednomian @f$n@f$
 * @return @f$m = n@f$
 */
bool MonoIEq(const Mono *m, const Mono *n){
        if(m->exp != n->exp){
            return false;
        }

        return PolyIsEq((const Poly *)&m->p, (const Poly*)&n->p);
}

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p = q@f$
 */
 //TODO poly is eq
bool PolyIsEq(const Poly *p, const Poly *q){
    if(PolyIsCoeff(p) && PolyIsCoeff(q)){
        return (p->coeff == q->coeff);
    }
    if(!PolyIsCoeff(p) && !PolyIsCoeff(q)){
        if(p->size == q->size){
            for(size_t i = 0; i < p->size; i++){
                if(!MonoIEq(&p->arr[i], &q->arr[i])){
                    return false;
                }
            }
            return true;
        }
        else{
            return false;
        }
    }

    return false;
}

/**
 * Porównuje wykładniki jednomianów.
 * @param p1 : wskaźnik na pierwszy jednomian do porównania
 * @param p2 : wskaźnik na drugi jednomian do porównania
 * @return : liczba ujemna gdy wykładnik pierwszego jednomianu jest mniejszy od drugiego,
 * 0 gdy wykładniki są równe, liczba dodatnia gdy wykładnik pierwszego wielomianu jest większy
 */
int compare_monos_by_exp(const void *p1, const void *p2) {
    Mono mono1 = *(Mono *)p1, mono2 = *(Mono *)p2;

    return mono1.exp - mono2.exp;
}

/**
 * Sortuje tablicę jednomianów po wykładnikach.
 * @param count : rozmiar tablicy jednomianów
 * @param monos : tablica jednomianów
 */
void sort_monos_by_exp(size_t count, Mono *monos) {
    qsort(monos, count, sizeof(Mono), compare_monos_by_exp);
}

/**
 * Zlicza różne wykładniki w tablicy jednomianów.
 * @param count : rozmiar tablicy jednomianów
 * @param monos : tablica jednomianów
 * @return
 */
unsigned count_different_exponents(size_t count, Mono *monos) {
    if (count == 0) {
        return 0;
    }
    unsigned result = 1;
    for (size_t i = 0; i < count - 1; i++) {
        if (monos[i].exp != monos[i + 1].exp) {
            result++;
        }
    }
    return result;
}

Mono add_monos(Mono *first, Mono *second);

/**
 * Sumuje tablicę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
// TODO poly add monos
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    // sortuje monos[] po exponencie:
    sort_monos_by_exp(count, (Mono *)monos);

    // tworze wielomian
    Poly new_poly;
    new_poly.size = count_different_exponents(count, (Mono *)monos);
    new_poly.arr = safe_calloc(new_poly.size, sizeof(Poly));
    size_t poly_i = 0, monos_i = 0;

    while(monos_i < count - 1){
        new_poly.arr[poly_i] = MonoClone(&monos[monos_i]);

        while(monos[monos_i].exp == monos[monos_i + 1].exp){
            new_poly.arr[poly_i] = add_monos(&new_poly.arr[poly_i], (Mono*) &monos[monos_i]);
            monos_i++;
        }

        poly_i++;
    }

    if(monos[count - 1].exp != monos[count - 2].exp){
        new_poly.arr[poly_i] = MonoClone(&monos[count - 1]);
    }

    // czyszcze pamięć po monos
    for (size_t j = 0; j < count; j++) {
        MonoDestroy((Mono *)&monos[j]);
    }

    return new_poly;
}

Poly add_coeff_to_poly(const Poly *p, const Poly *q) {
    assert(PolyIsCoeff(p));

    if (PolyIsZero(p)) {
        return PolyClone(p);
    }

    Poly new_poly;
    new_poly.size = count_different_exponents(q->size, q->arr) + 1;
    new_poly.arr = safe_malloc(new_poly.size * sizeof(Mono));
    new_poly.arr[0] = MonoFromPoly(p, 0);
    size_t i = 0;

    if (q->arr[0].exp == 0) { // pierwszy element z niestałego to c * x^0.
        i++;
        new_poly.arr[0].p.coeff += q->arr[0].p.coeff;
        new_poly.size--;
    }

    while (i < q->size) {
        new_poly.arr[i] = MonoClone(&q->arr[i]);
    }
    return new_poly;
}

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
// TODO poly add
Poly PolyAdd(const Poly *p, const Poly *q) {
    Poly new_poly = PolyZero();

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { // Obydwa są wielomianami stałymi.
        return PolyFromCoeff(p->coeff + q->coeff);
    } else if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) { // Obydwa są wielomianami niestałymi.
        new_poly.size = p->size + q->size;           // TODO is it ok?
        new_poly.arr = safe_malloc(new_poly.size * sizeof(Mono));
        size_t p_i = 0, q_i = 0, new_poly_i = 0;

        while (p_i < p->size && q_i < q->size) {
            if (p->arr[p_i].exp == q->arr[q_i].exp) {
                new_poly.arr[new_poly_i] = add_monos(&p->arr[p_i], &q->arr[q_i]);
                p_i++, q_i++;
            } else {
                if (p->arr[p_i].exp < q->arr[q_i].exp) {
                    new_poly.arr[new_poly_i] = MonoClone(&p->arr[p_i]);
                    p_i++;
                } else {
                    new_poly.arr[new_poly_i] = MonoClone(&q->arr[q_i]);
                    q_i++;
                }
            }
            new_poly_i++;
        }

        if (p_i < p->size) {
            while (p_i < p->size) {
                new_poly.arr[new_poly_i] = MonoClone(&p->arr[p_i]);
                p_i++, new_poly_i++;
            }
        } else if (q_i < q->size) {
            while (q_i < q->size) {
                new_poly.arr[new_poly_i] = MonoClone(&q->arr[q_i]);
                q_i++, new_poly_i++;
            }
        }
        new_poly.size = new_poly_i;
    } else {
        if (PolyIsCoeff(p)) {
            new_poly = add_coeff_to_poly(p, q);
        } else {
            new_poly = add_coeff_to_poly(q, p);
        }
    }

    return new_poly;
}

// dodaje dwa jednomiany, zwraca ich sume
Mono add_monos(Mono *first, Mono *second) {
    assert(first->exp == second->exp);

    return (Mono){.exp = first->exp,
                  .p = PolyAdd((const Poly *)&first->p, (const Poly *)&second->p)};
}

void PrintfPoly(const Poly *p, int i) {
    printf("(");
    if (p->arr[0].p.arr == NULL) {
        printf("%ld)x_%d^%d", p->arr[0].p.coeff, i, p->arr[0].exp);
    } else {
        PrintfPoly(&(p->arr[0].p), i + 1);
        printf(")x_%d^%d", i, p->arr[0].exp);
    }
    for (size_t j = 1; j < p->size; j++) {
        printf(" + (");
        if (p->arr[j].p.arr == NULL) {
            printf("%ld)x_%d^%d", p->arr[j].p.coeff, i, p->arr[j].exp);
        } else {
            PrintfPoly(&(p->arr[j].p), i + 1);
            printf(")x_%d^%d", i, p->arr[j].exp);
        }
    }
}
