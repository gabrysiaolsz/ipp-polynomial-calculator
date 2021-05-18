/** @file
 * Implementacja interfejsu parsera wejścia wczytującego wielomiany i polecenia.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "calc.h"
#include "errors.h"
#include "safe_memory_allocation.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

void IgnoreLine() {
    int c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

inputValue_t ReadUnsignedCoeff(unsigned long *result) {
    *result = 0;
    unsigned long longOverflow = (unsigned)LONG_MAX + 1;
    int c = getchar();
    if (!isdigit(c)) {
        IgnoreLine();
        return WRONG_POLY;
    }

    while (isdigit(c)) {
        *result = ((*result) * 10) + (c - '0');
        if (*result > longOverflow) {
            IgnoreLine();
            return WRONG_POLY;
        }
        c = getchar();
    }
    if (c == EOF) {
        return NO_ERROR;
    }
    ungetc(c, stdin);
    return NO_ERROR;
}

inputValue_t ReadExp(unsigned int *result) {
    *result = 0;
    int c = getchar();
    while (isdigit(c)) {
        *result = ((*result) * 10) + (c - '0');
        if (*result > INT_MAX) {
            IgnoreLine();
            return WRONG_POLY;
        }
        c = getchar();
    }
    if (c == EOF) {
        return ENCOUNTERED_EOF;
    }
    ungetc(c, stdin);
    return NO_ERROR;
}

inputValue_t ReadConstPoly(Poly *result, bool isNegative);

inputValue_t ReadMono(Mono *result) {
    Poly p;
    int c = getchar();
    inputValue_t error;
    if (c == '(') {
        Mono m;
        error = ReadMono(&m);
        if (error != NO_ERROR) {
            IgnoreLine();
            return error;
        }
    }
    else if (c == EOF) {
        return WRONG_POLY;
    } else if (c == '-') {
        error = ReadConstPoly(&p, true);
        if(error != NO_ERROR){
            IgnoreLine();
            return error;
        }
    } else {
        ungetc(c, stdin);
        error = ReadConstPoly(&p, false);
        if(error != NO_ERROR){
            IgnoreLine();
            return error;
        }
    }

    c = getchar();
    if (c != ',') {
        IgnoreLine();
        return WRONG_POLY;
    }

    unsigned int exp;
    error = ReadExp(&exp);
    if (error != NO_ERROR) {
        IgnoreLine();
        return error;
    }
    if (exp < 0 || exp > INT_MAX) {
        IgnoreLine();
        return WRONG_POLY;
    }

    c = getchar();
    if (c != ')') {
        IgnoreLine();
        return WRONG_POLY;
    }

    *result = MonoFromPoly(&p, (int)exp);
    if (RecursiveMonoIsZero(result)) {
        MonoDestroy(result);
        PolyDestroy(&p);
        *result = (Mono){.exp = (int)exp, .p = PolyZero()};
    }

    return NO_ERROR;
}

inputValue_t ReadPoly(Poly *result) {
    Mono m1, m2;
    inputValue_t error = ReadMono(&m1);
    if (error != NO_ERROR) {
        IgnoreLine();
        return error;
    }

    int c = getchar();
    if (c == '+') {
        getchar();
        error = ReadMono(&m2);
        if(error != NO_ERROR){
            IgnoreLine();
            return error;
        }
        Mono *monos = SafeMalloc(2 * sizeof(Mono));
        monos[0] = m1;
        monos[1] = m2;
        *result = PolyAddMonos(2, monos);
    } else {
        if (PolyIsZero(&m1.p)) {
            *result = PolyZero();
        }
        *result = (Poly){.arr = SafeMalloc(sizeof(Mono)), .size = 1};
        result->arr[0] = m1;
        MonoDestroy(&m1);
    }

    return NO_ERROR;
}

inputValue_t ReadConstPoly(Poly *result, bool isNegative) {
    unsigned long coeff;
    inputValue_t error = ReadUnsignedCoeff(&coeff);
    if(error != NO_ERROR){
        return error;
    }
    int c = getchar();
    if (c != '\n' && c != EOF) {
        IgnoreLine();
        return WRONG_POLY;
    }

    if (isNegative) {
        if(coeff - 1 > LONG_MAX){
            IgnoreLine();
            return WRONG_POLY;
        }
        *result = PolyFromCoeff(-1 * (long)coeff);
    } else {
        if(coeff > LONG_MAX){
            IgnoreLine();
            return WRONG_POLY;
        }
        *result = PolyFromCoeff((long)coeff);
    }

    return NO_ERROR;
}

inputValue_t ReadOrder() {
    errno = 0;
    char word[30];
    scanf("%s", word);
    if (errno != 0) {
        IgnoreLine();
        return WRONG_ORDER;
    } else {
        printf("wczytane słowo: %s\n", word);
        return VALID_ORDER;
    }
}

inputValue_t ReadOneLineOfInput() {
    inputValue_t error;
    Poly p;
    int c = getchar();
    switch (c) {
        case '#':
            IgnoreLine();
            return LINE_IGNORED;
        case '(':
            error = ReadPoly(&p);
            return error;
        case EOF:
            return ENCOUNTERED_EOF;
        case '\n':
            printf("nowa linia!!\n");
            return LINE_IGNORED;
        default:
            if (isdigit(c)) {
                ungetc(c, stdin);
                error = ReadConstPoly(&p, false);
                return error;
            } else if (c == '-') {
                error = ReadConstPoly(&p, true);
                return error;
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                return ReadOrder();
            } else {
                printf("nie pasuje\n");
                return WRONG_POLY;
            }
    }
}

//TODO order