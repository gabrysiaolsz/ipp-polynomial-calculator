#include "poly.h"
#include "safe_memory_allocation.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
// TODO poly destroy
void PolyDestroy(Poly *p) {

    for (size_t i = 0; i < p->size; i++) {
    }
}

int compare_monos(const void *p1, const void *p2){
    Mono mono1 = *(Mono*)p1, mono2 = *(Mono*)p2;

    return mono1.exp - mono2.exp;
}

void sort_mono_array(size_t count, Mono *monos){
    qsort(monos, count, sizeof(Mono), compare_monos);
}

unsigned count_different_exponents(size_t count, Mono *monos){
    if(count == 0){
        return 0;
    }
    unsigned result = 1;
    for(size_t i = 0; i < count - 1; i++){
        if(monos[i].exp != monos[i+1].exp){
            result++;
        }
    }
    return result;
}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
//TODO poly add monos
Poly PolyAddMonos(size_t count, const Mono monos[]){
    //sortuje monos[] po exponencie:
    sort_mono_array(count, (Mono*) monos);

    //zliczam ile jest różnych exp
    unsigned number_of_different_exponents = count_different_exponents(count, (Mono*) monos);

    //tworze wielomian
    Poly new_poly;
    new_poly.size = number_of_different_exponents;
    new_poly.arr = safe_calloc(new_poly.size, sizeof(Poly));

    

    return new_poly;
}


void PrintfPoly(const Poly *p, int exp, int i) {
    printf("(");
    if (p->arr[0].p.arr == NULL) {
        printf("%ld)x_%d^%d", p->arr[0].p.coeff, i, p->arr[0].exp);
    }
    else {
        PrintfPoly(&(p->arr[0].p), p->arr[0].exp, i+1);
        printf(")x_%d^%d", i, p->arr[0].exp);
    }
    for (size_t j = 1; j < p->size; j++) {
        printf(" + (");
        if (p->arr[j].p.arr == NULL) {
            printf("%ld)x_%d^%d", p->arr[j].p.coeff, i, p->arr[j].exp);
        }
        else {
            PrintfPoly(&(p->arr[j].p), p->arr[j].exp, i+1);
            printf(")x_%d^%d", i, p->arr[j].exp);
        }
    }
}
