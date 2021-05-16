/** @file
 * Implementacja interfejsu parsera wejścia wczytującego wielomiany i polecenia.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include "calc.h"
#include "safe_memory_allocation.h"

void IgnoreLine() {
    int c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

unsigned long ReadUnsignedCoeff() {
    unsigned long coeff = 0;
    int c = getchar();
    while (isdigit(c)) {
        coeff = (coeff * 10) + (c - '0');
        c = getchar();
    }
    ungetc(c, stdin);
    return coeff;
}

poly_exp_t ReadExp() {
    poly_exp_t exp = 0;
    int c = getchar();
    while (isdigit(c)) {
        exp = (exp * 10) + (c - '0');
        c = getchar();
    }
    ungetc(c, stdin);

    return exp;
}

Mono ReadMono() {
    Poly p;

    int c = getchar();
    if (c == '(') {
        Mono m = ReadMono();
        p = (Poly){.arr = SafeMalloc(sizeof(Poly)), .size = 1};
        p.arr[0] = m;
    } else {
        ungetc(c, stdin);
        poly_coeff_t coeff = ReadUnsignedCoeff();
        p = PolyFromCoeff(coeff);
    }

    c = getchar();
    if (c != ',') {
        printf("niepoprawny format\n");
    }

    poly_exp_t exp = ReadExp();
    c = getchar();
    if (c == ')') {
        printf("skończyliśmy :)\n");
    } else {
        printf("lol błąd\n");
    }

    Mono result = MonoFromPoly(&p, exp);
    if (RecursiveMonoIsZero(&result)) {
        MonoDestroy(&result);
        PolyDestroy(&p);
        return (Mono){.exp = exp, .p = PolyZero()};
    }

    return MonoFromPoly(&p, exp);
}

Poly ReadPoly() {
    Poly p;
    Mono m1 = ReadMono();
    int c = getchar();
    if (c == '+') {
        printf("mamy wielomian z plusem!!!");
        getchar();
        Mono m2 = ReadMono();
        Mono *monos = SafeMalloc(2 * sizeof(Mono));
        monos[0] = m1;
        monos[1] = m2;
        p = PolyAddMonos(2, monos);
    } else {
        if (RecursiveMonoIsZero(&m1)) {
            MonoDestroy(&m1);
            return PolyZero();
        }
        p = (Poly){.arr = SafeMalloc(sizeof(Mono)), .size = 1};
        p.arr[0] = m1;
        MonoDestroy(&m1);
    }

    return p;
}

Poly ReadConstPoly(bool isNegative) {
    poly_coeff_t coeff = ReadUnsignedCoeff();
    if (isNegative) {
        return PolyFromCoeff(-1 * coeff);
    } else {
        return PolyFromCoeff(coeff);
    }
}

void ReadOrder() {
    errno = 0;
    char word[30];
    scanf("%s", word);
    if (errno != 0) {
        printf("niepoprawne dane\n");
    } else {
        printf("wczytane słowo: %s\n", word);
    }
}

void ReadOneLineOfInput() {
    int c = getchar();
    switch (c) {
        case '#':
            IgnoreLine();
            break;
        case '(':
            ReadPoly();
            break;
        case EOF:
            printf("end of file\n");
            return;
        case '\n':
            printf("nowa linia\n");
            return;
        default:
            if (isdigit(c)) {
                ReadConstPoly(false);
            } else if (c == '-') {
                ReadConstPoly(true);
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                ReadOrder();
            } else {
                printf("nie pasuje\n");
            }
    }
}