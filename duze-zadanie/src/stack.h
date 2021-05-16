/** @file
 * Interfejs stosu.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef POLYNOMIALS_STACK_H
#define POLYNOMIALS_STACK_H

#include "poly.h"
#include "safe_memory_allocation.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * Struktura reprezentująca stos.
 */
typedef struct Stack {
    // Zmienna przechowująca aktualny rozmiar stosu.
    size_t size;
    // Zmienna przechowująca informację o zaalokowanym rozmiarze stosu.
    size_t capacity;
    // Tablica przechowująca elementy stosu.
    Poly *array;
} Stack;

/**
 * Tworzy stos. Ustawia jego capacity na podany w argumentach rozmiar,
 * wartość top ustawia na -1, alokuje pamięć na tablicę rozmiaru capacity.
 * @param capacity : rozmiar stosu.
 * @return wskaźnik na stos.
 */
Stack *CreateStack();

/**
 * Zwiększa rozmiar stosu.
 * @param stack : stos,
 * @return wskaźnik na powiększony stos.
 */
void ResizeStack(Stack *stack);

/**
 * Sprawdza, czy stos jest pusty.
 * @param stack : stos
 * @return true, jeśli stos jest pusty i false w przeciwnym przypadku.
 */
static inline bool IsEmpty(Stack *stack) {
    return stack->size == 0;
}

/**
 * Sprawdza, czy stos jest pełny.
 * @param stack : stos
 * @return true, jeśli stos jest pełny i false w przeciwnym przypadku.
 */
bool IsFull(Stack *stack);

/**
 * Sprawdza rozmiar stosu.
 * @param stack : stos
 * @return rozmiar stosu.
 */
static inline size_t StackSize(Stack *stack) {
    return stack->size;
}

/**
 * Dodaje wielomian na szczyt stosu. Inkrementuje top o jeden.
 * Jeśli stos jest zapełniony, zwiększa go.
 * @param stack : stos
 * @param p : wielomian do dodania
 */
void Push(Stack *stack, Poly p);

/**
 * Ściąga element z góry stosu.
 * Wywołanie funkcji na pustym stosie to błąd.
 * @param stack : stos,
 * @return wielomian ściągnięty z góry stosu.
 */
Poly Pop(Stack *stack);

/**
 * Zwraca ostatni element stosu.
 * @return element na górze stosu.
 */
Poly Peek(Stack *stack);

/**
 * Usuwa stos i czyści pamięć po wielomianach które się w nim znajdują.
 */
void DestroyStack(Stack *stack);

#endif // POLYNOMIALS_STACK_H
