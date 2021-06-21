/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * W opisach funkcji zakładamy, że "wyjście" odnosi się do "standardowego wyjścia", natomiast
 * informacja o wypisywaniu na standardowe wyjście diagnostyczne jest wyraźnie zaznaczona.
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "errors.h"
#include "input_parser.h"
#include "stack.h"
#include <stdio.h>
#include <string.h>

/**
 * Wypisuje na standardowe wyjście diagnostyczne informację o braku wystarczającej liczby
 * wielomianów do wykonania polecenia wraz z numer linii na której było to polecenie.
 * @param lineNumber : numer linii na której znajdowało się polecenie.
 */
static inline void PrintStackUnderflow(unsigned int lineNumber) {
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
}

/**
 * Wypisuje 0 albo 1.
 * @param x : zmienna którą mamy wypisać.
 */
static inline void PrintBool(bool x) {
    printf("%d\n", x);
}

/**
 * Wypisuje współczynnik wielomianu.
 * @param p : wielomian, którego współczynnik wypisujemy.
 */
static inline void PrintCoeff(Poly p) {
    printf("%ld", p.coeff);
}

static inline void PrintPoly(Poly p);

/**
 * Wypisuje jednomian.
 * @param m : jednomian do wypisania.
 */
static inline void PrintMono(Mono m) {
    PrintPoly(m.p);
    printf(",%d", m.exp);
}

/**
 * Wypisuje wielomian, którego rozmiar wynosi 1.
 * @param p : wielomian rozmiaru 1 do wypisania.
 */
static inline void PrintPolyWithSize1(Poly p) {
    poly_coeff_t tmp;
    if (RecursivePolyIsCoeff(&p.arr[0].p, &tmp)) {
        PrintPoly(p.arr[0].p);
    } else {
        printf("(");
        PrintPoly(p.arr[0].p);
        printf(",0)");
    }
}

/**
 * Wypisuje wielomian.
 * @param p : wielomian do wypisania.
 */
static inline void PrintPoly(Poly p) {
    if (!PolyIsCoeff(&p)) {
        if (p.size == 1 && p.arr[0].exp == 0) {
            PrintPolyWithSize1(p);
            return;
        }
        for (size_t i = 0; i < p.size; i++) {
            if (i > 0) {
                printf("+");
            }
            printf("(");
            PrintMono(p.arr[i]);
            printf(")");
        }
    } else {
        PrintCoeff(p);
    }
}

/**
 * Wykonuje polecenie ZERO.
 * @param stack : stos wielomianów.
 */
static inline void ExecuteZero(Stack *stack) {
    Push(stack, PolyZero());
}

/**
 * Wykonuje polecenie IS_ZERO albo IS_COEFF, w zależności od argumentu.
 * @param stack : stos wielomianów.
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 * @param function : wskaźnik na funkcję PolyIsCoeff albo PolyIsZero, którą będziemy wykonywać.
 */
static inline void ExecuteIs(Stack *stack, unsigned int lineNumber,
                             bool (*function)(const Poly *)) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        if (function(&peekPoly)) {
            PrintBool(1);
        } else {
            PrintBool(0);
        }
    }
}

/**
 * Wykonuje polecenie CLONE.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteClone(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        Poly result = PolyClone(&peekPoly);
        Push(stack, result);
    }
}

/**
 * Wykonuje polecenie ADD, SUB lub MUL.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie,
 * @param function : wskaźnik na funkcję PolyAdd, PolySub albo PolyMul, którą będziemy wykonywać.
 */
static inline void ExecuteArithmeticOp(Stack *stack, unsigned int lineNumber,
                                       Poly (*function)(const Poly *, const Poly *)) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Pop(stack);
        Poly result = function(&first, &second);
        PolyDestroy(&first);
        PolyDestroy(&second);
        Push(stack, result);
    }
}

/**
 * Wykonuje polecenie IS_EQ.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteIsEq(Stack *stack, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Peek(stack);
        if (PolyIsEq(&first, &second)) {
            PrintBool(1);
        } else {
            PrintBool(0);
        }
        Push(stack, first);
    }
}

/**
 * Wykonuje polecenie NEG.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteNeg(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        Push(stack, PolyNeg(&poly));
        PolyDestroy(&poly);
    }
}

/**
 * Wykonuje polecenie DEG.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteDeg(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        poly_exp_t exp = PolyDeg(&peekPoly);
        printf("%d\n", exp);
    }
}

/**
 * Wykonuje polecenie DEG_BY.
 * @param stack : stos wielomianów,
 * @param parameter : parametr polecenia,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteDegBy(Stack *stack, size_t parameter, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        poly_exp_t exp = PolyDegBy(&peekPoly, parameter);
        printf("%d\n", exp);
    }
}

/**
 * Wykonuje polecenie AT.
 * @param stack : stos wielomianów,
 * @param parameter : parametr polecenia,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteAt(Stack *stack, poly_coeff_t parameter, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        Poly result = PolyAt(&poly, parameter);
        PolyDestroy(&poly);
        Push(stack, result);
    }
}

/**
 * Wykonuje polecenie COMPOSE.
 * @param stack : stos wielomianów,
 * @param parameter : parametr polecenia,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecuteCompose(Stack *stack, size_t parameter, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if ((size > 0 && size - 1 < parameter) || size == 0) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly p = Pop(stack);
        Poly *array = SafeMalloc(parameter * sizeof(Poly));
        for (size_t i = 0; i < parameter; i++) {
            Poly q = Pop(stack);
            array[parameter - 1 - i] = q;
        }
        Poly result = PolyCompose(&p, parameter, array);
        PolyDestroy(&p);
        for (size_t i = 0; i < parameter; i++) {
            PolyDestroy(&array[i]);
        }
        free(array);
        Push(stack, result);
    }
}

/**
 * Wykonuje polecenie PRINT.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecutePrint(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        PrintPoly(peekPoly);
        printf("\n");
    }
}

/**
 * Wykonuje polecenie POP.
 * @param stack : stos wielomianów,
 * @param lineNumber : numer linii na której wystąpiło polecenie.
 */
static inline void ExecutePop(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        PolyDestroy(&poly);
    }
}

/**
 * Sprawdza, czy nazwa polecenia jest poprawna - jeśli tak, to je wykonuje. W przeciwnym przypadku
 * wypisuje na standardowe wyjście diagnostyczne komunikat o błędzie.
 * @param stack : stos wielomianów,
 * @param command : polecenie,
 * @param lineNumber : numer linii.
 */
static inline void ExecuteCommand(Stack *stack, Command command, unsigned int lineNumber) {
    if (strcmp(command.name, "ZERO") == 0) {
        ExecuteZero(stack);
    } else if (strcmp(command.name, "IS_COEFF") == 0) {
        ExecuteIs(stack, lineNumber, PolyIsCoeff);
    } else if (strcmp(command.name, "IS_ZERO") == 0) {
        ExecuteIs(stack, lineNumber, PolyIsZero);
    } else if (strcmp(command.name, "CLONE") == 0) {
        ExecuteClone(stack, lineNumber);
    } else if (strcmp(command.name, "ADD") == 0) {
        ExecuteArithmeticOp(stack, lineNumber, PolyAdd);
    } else if (strcmp(command.name, "MUL") == 0) {
        ExecuteArithmeticOp(stack, lineNumber, PolyMul);
    } else if (strcmp(command.name, "NEG") == 0) {
        ExecuteNeg(stack, lineNumber);
    } else if (strcmp(command.name, "SUB") == 0) {
        ExecuteArithmeticOp(stack, lineNumber, PolySub);
    } else if (strcmp(command.name, "IS_EQ") == 0) {
        ExecuteIsEq(stack, lineNumber);
    } else if (strcmp(command.name, "DEG") == 0) {
        ExecuteDeg(stack, lineNumber);
    } else if (strcmp(command.name, "DEG_BY") == 0) {
        ExecuteDegBy(stack, command.degByOrComposeParameter, lineNumber);
    } else if (strcmp(command.name, "AT") == 0) {
        ExecuteAt(stack, command.atParameter, lineNumber);
    } else if (strcmp(command.name, "PRINT") == 0) {
        ExecutePrint(stack, lineNumber);
    } else if (strcmp(command.name, "POP") == 0) {
        ExecutePop(stack, lineNumber);
    } else if (strcmp(command.name, "COMPOSE") == 0) {
        ExecuteCompose(stack, command.degByOrComposeParameter, lineNumber);
    } else {
        fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineNumber);
    }
}

/**
 * Wstawia na stos wielomian.
 * @param stack : stos wielomianów,
 * @param p : wielomian.
 */
static inline void PushPoly(Stack *stack, Poly p) {
    Push(stack, p);
}

/**
 * Wykonuje dane wejściowe programu. Czyta po linijce danych wejściowych i w zależności od tego,
 * czy była ona poprawna oraz co zawierała podejmuje odpowiednie działanie.
 * @param stack : stos wielomianów.
 */
void ExecuteInput(Stack *stack) {
    unsigned int lineNumber = 1;
    ParsedLine line;
    while (true) {
        error_t error = ReadOneLineOfInput(&line);
        switch (error) {
            case NO_ERROR:
                if (line.isPoly) {
                    PushPoly(stack, line.poly);
                } else {
                    ExecuteCommand(stack, line.command, lineNumber);
                }
                break;
            case INVALID_VALUE:
                if (line.isPoly) {
                    fprintf(stderr, "ERROR %d WRONG POLY\n", lineNumber);
                } else {
                    fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineNumber);
                }
                break;
            case LINE_IGNORED:
                break;
            case DEG_BY_ERROR:
                fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", lineNumber);
                break;
            case AT_ERROR:
                fprintf(stderr, "ERROR %d AT WRONG VALUE\n", lineNumber);
                break;
            case COMPOSE_ERROR:
                fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", lineNumber);
                break;
            case ENCOUNTERED_EOF:
                return;
                // Nie mamy przypadku "default", ponieważ sprawdzamy każdą możliwość typu
                // wyliczeniowego
        }
        lineNumber++;
    }
}

/**
 * Główna funkcja wykonująca cały program.
 */
 int main() {
     Stack *stack = CreateStack();
     ExecuteInput(stack);
     DestroyStack(stack);
 }
