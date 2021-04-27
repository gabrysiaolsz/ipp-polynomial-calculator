#ifndef SAFE_MEMORY_ALLOCATION_H
#define SAFE_MEMORY_ALLOCATION_H

#include <stdlib.h>

// Checks whether malloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what malloc has returned.
static inline void *SafeMalloc(size_t size) {
    void *malloc_ptr = malloc(size);
    if (malloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return malloc_ptr;
}

// Checks whether realloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what realloc has returned.
static inline void *SafeRealloc(void *ptr, size_t size) {
    void *realloc_ptr = realloc(ptr, size);
    if (realloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return realloc_ptr;
}

// Checks whether calloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what calloc has returned.
static inline void *SafeCalloc(size_t number_of_elements, size_t size) {
    void *calloc_ptr = calloc(number_of_elements, size);
    if (calloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return calloc_ptr;
}

#endif // SAFE_MEMORY_ALLOCATION