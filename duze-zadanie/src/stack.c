/** @file
 * Interfejs stosu przechowującego wielomiany rzadkie wielu zmiennych.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "stack.h"

#define STARTING_CAPACITY 32

Stack *CreateStack() {
    Stack *stack = SafeMalloc(sizeof(Stack));
    stack->capacity = STARTING_CAPACITY;
    stack->size = 0;
    stack->array = SafeMalloc(stack->capacity * sizeof(Poly));
    return stack;
}

void ResizeStack(Stack *stack) {
    size_t newCapacity = stack->capacity * 2;

    stack->array = SafeRealloc(stack->array, newCapacity * sizeof(Poly));
    stack->capacity = newCapacity;
}

bool IsFull(Stack *stack) {
    return stack->size == stack->capacity;
}

void Push(Stack *stack, Poly p) {
    if (IsFull(stack)) {
        ResizeStack(stack);
    }
    stack->array[stack->size++] = p;
}

Poly Pop(Stack *stack) {
    assert(!IsEmpty(stack));
    return stack->array[--stack->size];
}

Poly Peek(Stack *stack) {
    assert(!IsEmpty(stack));
    return stack->array[stack->size - 1];
}

void DestroyStack(Stack *stack) {
    for (size_t i = 0; i < stack->size; i++) {
        PolyDestroy(&stack->array[i]);
    }
    free(stack->array);
    free(stack);
}
