/** @file
 * Interfejs stosu przechowującego wielomiany rzadkie wielu zmiennych.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "stack.h"

Stack *CreateStack(size_t capacity) {
    Stack *stack = SafeMalloc(sizeof(Stack));
    stack->capacity = capacity;
    stack->size = 0;
    stack->array = SafeMalloc(stack->capacity * sizeof(Poly));
    return stack;
}

Stack *ResizeStack(Stack *stack) {
    size_t newCapacity = (stack->capacity + 1) * 2;
    // TODO sprawdzić czy nie było overflow

    stack->array = SafeRealloc(stack, newCapacity);
    stack->capacity = newCapacity;

    return stack;
}

void Push(Stack *stack, const Poly *p) {
    if (IsFull(stack)) {
        stack = ResizeStack(stack);
    }
    stack->array[++stack->size] = *p;
}

Poly Pop(Stack *stack) {
    assert(!IsEmpty(stack));
    Poly p = stack->array[stack->size - 1];
    stack->size--;
    return p;
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
