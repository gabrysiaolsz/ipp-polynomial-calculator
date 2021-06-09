/** @file
 * Interfejs obsługujący błędy związane z alokacją pamięci.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef SAFE_MEMORY_ALLOCATION_H
#define SAFE_MEMORY_ALLOCATION_H

#include <stdlib.h>

/**
 * Sprawdza, czy malloc nie zwrócił błędu.
 * @param size : rozmiar pamięci do zaalokowania,
 * @return jeśli malloc zwrócił NULL, kończy program z błędem 1. W przeciwnym
 * przypadku zwraca to, co zwrócił malloc.
 */
static inline void *SafeMalloc(size_t size) {
    void *malloc_ptr = malloc(size);
    if (malloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return malloc_ptr;
}

/**
 * Sprawdza, czy realloc nie zwrócił błędu.
 * @param size : rozmiar pamięci do zaalokowania,
 * @return jeśli malloc zwrócił NULL, kończy program z błędem 1. W przeciwnym
 * przypadku zwraca to, co zwrócił malloc.
 */
static inline void *SafeRealloc(void *ptr, size_t size) {
    void *realloc_ptr = realloc(ptr, size);
    if (realloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return realloc_ptr;
}

#endif // SAFE_MEMORY_ALLOCATION