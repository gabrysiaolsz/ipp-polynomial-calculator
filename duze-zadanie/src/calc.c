/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "errors.h"
#include "input_parser.h"
#include "stack.h"
#include <stdio.h>
#include <string.h>

void PrintStackUnderflow(unsigned int lineNumber) {
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
}

void PrintOne() {
    printf("1\n");
}

void PrintZero() {
    printf("0\n");
}

void PrintCoeff(Poly p){
    printf("%ld", p.coeff);
}

void PrintPoly(Poly p);

void PrintMono(Mono m){
    PrintPoly(m.p);
    printf(",%d", m.exp);
}

void PrintPoly(Poly p) {
    if(!PolyIsCoeff(&p)){
        if(p.size == 1 && p.arr[0].exp == 0){
            poly_coeff_t tmp;
            if(RecursivePolyIsCoeff(&p.arr[0].p, &tmp)){
                PrintPoly(p.arr[0].p);
                return;
            }
            else{
                printf("(");
                PrintPoly(p.arr[0].p);
                printf(",0)");
                return;
            }

        }
        for(size_t i = 0; i < p.size; i++){
            if(i > 0){
                printf("+");
            }
            printf("(");
            PrintMono(p.arr[i]);
            printf(")");
        }
    }
    else{
        PrintCoeff(p);
    }
}

void ExecuteZero(Stack *stack) {
    Push(stack, PolyZero());
}

void ExecuteIsCoeff(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
        PrintZero();
    } else {
        Poly peekPoly = Peek(stack);
        if (PolyIsCoeff(&peekPoly)) {
            PrintOne();
        } else {
            PrintZero();
        }
    }
}

void ExecuteIsZero(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
        PrintZero();
    } else {
        Poly peekPoly = Peek(stack);
        if (PolyIsZero(&peekPoly)) {
            PrintOne();
        } else {
            PrintZero();
        }
    }
}

void ExecuteClone(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        Poly result = PolyClone(&peekPoly);
        Push(stack, result);
    }
}

// TODO merge sub, add, mul

void ExecuteAdd(Stack *stack, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Pop(stack);
        Poly result = PolyAdd(&first, &second);
        PolyDestroy(&first);
        PolyDestroy(&second);
        Push(stack, result);
    }
}

void ExecuteMul(Stack *stack, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Pop(stack);
        Poly result = PolyMul(&first, &second);
        PolyDestroy(&first);
        PolyDestroy(&second);
        Push(stack, result);
    }
}

void ExecuteNeg(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        Push(stack, PolyNeg(&poly));
        PolyDestroy(&poly);
    }
}

void ExecuteSub(Stack *stack, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Pop(stack);
        Poly result = PolySub(&first, &second);
        PolyDestroy(&first);
        PolyDestroy(&second);
        Push(stack, result);
    }
}

void ExecuteIsEq(Stack *stack, unsigned int lineNumber) {
    size_t size = StackSize(stack);
    if (size < 2) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly first = Pop(stack);
        Poly second = Peek(stack);
        if (PolyIsEq(&first, &second)) {
            PrintOne();
        } else {
            PrintZero();
        }
        Push(stack, first);
    }
}

void ExecuteDeg(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        poly_exp_t exp = PolyDeg(&peekPoly);
        printf("%d\n", exp);
    }
}

void ExecuteDegBy(Stack *stack, size_t parameter, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        poly_exp_t exp = PolyDegBy(&peekPoly, parameter);
        printf("%d\n", exp);
    }
}

void ExecuteAt(Stack *stack, poly_coeff_t parameter, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        Poly result = PolyAt(&poly, parameter);
        PolyDestroy(&poly);
        Push(stack, result);
    }
}

void ExecutePrint(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly peekPoly = Peek(stack);
        PrintPoly(peekPoly);
        printf("\n");
    }
}

void ExecutePop(Stack *stack, unsigned int lineNumber) {
    if (IsEmpty(stack)) {
        PrintStackUnderflow(lineNumber);
    } else {
        Poly poly = Pop(stack);
        PolyDestroy(&poly);
    }
}

void ExecuteCommand(Stack *stack, Command command, unsigned int lineNumber) {
    if (strcmp(command.name, "ZERO") == 0) {
        ExecuteZero(stack);
    } else if (strcmp(command.name, "IS_COEFF") == 0) {
        ExecuteIsCoeff(stack, lineNumber);
    } else if (strcmp(command.name, "IS_ZERO") == 0) {
        ExecuteIsZero(stack, lineNumber);
    } else if (strcmp(command.name, "CLONE") == 0) {
        ExecuteClone(stack, lineNumber);
    } else if (strcmp(command.name, "ADD") == 0) {
        ExecuteAdd(stack, lineNumber);
    } else if (strcmp(command.name, "MUL") == 0) {
        ExecuteMul(stack, lineNumber);
    } else if (strcmp(command.name, "NEG") == 0) {
        ExecuteNeg(stack, lineNumber);
    } else if (strcmp(command.name, "SUB") == 0) {
        ExecuteSub(stack, lineNumber);
    } else if (strcmp(command.name, "IS_EQ") == 0) {
        ExecuteIsEq(stack, lineNumber);
    } else if (strcmp(command.name, "DEG") == 0) {
        ExecuteDeg(stack, lineNumber);
    } else if (strcmp(command.name, "DEG_BY") == 0) {
        ExecuteDegBy(stack, command.degByParameter, lineNumber);
    } else if (strcmp(command.name, "AT") == 0) {
        ExecuteAt(stack, command.atParameter, lineNumber);
    } else if (strcmp(command.name, "PRINT") == 0) {
        ExecutePrint(stack, lineNumber);
    } else if (strcmp(command.name, "POP") == 0) {
        ExecutePop(stack, lineNumber);
    } else {
        fprintf(stderr, "ERROR %d WRONG COMMAND\n", lineNumber);
    }
}

void PushPoly(Stack *stack, Poly p) {
    Push(stack, p);
}

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
                if(line.isPoly){
                    fprintf(stderr, "ERROR %d WRONG POLY\n", lineNumber);
                }
                else{
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
            case ENCOUNTERED_EOF:
                return;
            default:;
        }
        lineNumber++;
    }
}

int main() {
    Stack *stack = CreateStack();
    ExecuteInput(stack);
    DestroyStack(stack);
}