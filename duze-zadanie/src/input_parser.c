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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void IgnoreLine() {
    int c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

error_t ReadUnsignedCoeff(unsigned long *result) {
    *result = 0;
    unsigned long longOverflow = (unsigned long)LONG_MAX + 1;
    int c = getchar();
    if (!isdigit(c)) {
        IgnoreLine();
        return INVALID_VALUE;
    }

    while (isdigit(c)) {
        *result = ((*result) * 10) + (c - '0');
        if (*result > longOverflow) {
            IgnoreLine();
            return INVALID_VALUE;
        }
        c = getchar();
    }

    if (c == EOF) {
        return NO_ERROR;
    }
    ungetc(c, stdin);
    return NO_ERROR;
}

error_t ReadExp(unsigned int *result) {
    *result = 0;
    int c = getchar();
    while (isdigit(c)) {
        *result = ((*result) * 10) + (c - '0');
        if (*result > INT_MAX) {
            IgnoreLine();
            return INVALID_VALUE;
        }
        c = getchar();
    }

    if (c == EOF) {
        return ENCOUNTERED_EOF;
    }
    ungetc(c, stdin);
    return NO_ERROR;
}

error_t ReadConstPoly(Poly *result, bool isNegative);

error_t ReadMono(Mono *result) {
    Poly p;
    int c = getchar();
    error_t error;

    if (c == '(') {
        Mono m;
        error = ReadMono(&m);
        if (error != NO_ERROR) {
            IgnoreLine();
            MonoDestroy(&m); // TODO chyba tego nie potrzebuje
            return error;
        }
    } else if (c == EOF) {
        return ENCOUNTERED_EOF;
    } else if (c == '-') {
        error = ReadConstPoly(&p, true);
        if (error != NO_ERROR) {
            IgnoreLine();
            return error;
        }
    } else {
        ungetc(c, stdin);
        error = ReadConstPoly(&p, false);
        if (error != NO_ERROR) {
            IgnoreLine();
            return error;
        }
    }

    c = getchar();
    if (c != ',') {
        IgnoreLine();
        return INVALID_VALUE;
    }

    unsigned int exp;
    error = ReadExp(&exp);
    if (error != NO_ERROR) {
        return error;
    }

    c = getchar();
    if (c != ')') {
        IgnoreLine();
        return INVALID_VALUE;
    }

    *result = MonoFromPoly(&p, (int)exp);
    if (RecursiveMonoIsZero(result)) {
        MonoDestroy(result); // TODO chyba tego nie potrzebuje
        *result = (Mono){.exp = (int)exp, .p = PolyZero()};
    }

    return NO_ERROR;
}

Poly AddMonoToPoly(Poly *p, Mono *m) {
    if (PolyIsZero(&m->p)) {
        return *p;
    }
    if (PolyIsZero(p)) {
        Poly polyResult = {.size = 1, .arr = SafeMalloc(sizeof(Mono))};
        polyResult.arr[0] = *m;
        return polyResult;
    }

    Poly tmpPoly = {.size = 1, .arr = SafeMalloc(sizeof(Mono))};
    tmpPoly.arr[0] = *m;
    return PolyAdd(p, &tmpPoly);
}

error_t ReadPoly(Poly *polyResult) {
    Mono tmpMono;
    error_t error = ReadMono(&tmpMono);
    if (error != NO_ERROR) {
        return error;
    }

    if (PolyIsZero(&tmpMono.p)) {
        *polyResult = PolyZero();
    } else {
        *polyResult = (Poly){.size = 1, .arr = SafeMalloc(sizeof(Mono))};
        polyResult->arr[0] = tmpMono;
    }

    int c = getchar();
    while (c == '+') {
        error = ReadMono(&tmpMono);
        if (error != NO_ERROR) {
            PolyDestroy(polyResult);
            return error;
        }
        *polyResult = AddMonoToPoly(polyResult, &tmpMono);
        c = getchar();
    }

    if (c == '\n' || c == EOF) {
        return NO_ERROR;
    }

    PolyDestroy(polyResult);
    return INVALID_VALUE;
}

error_t ReadConstPoly(Poly *result, bool isNegative) {
    unsigned long coeff;
    error_t error = ReadUnsignedCoeff(&coeff);
    if (error != NO_ERROR) {
        return error;
    }

    if (isNegative) {
        *result = PolyFromCoeff(-1 * (long)coeff);
    } else {
        if (coeff > LONG_MAX) {
            IgnoreLine();
            return INVALID_VALUE;
        }
        *result = PolyFromCoeff((long)coeff);
    }

    return NO_ERROR;
}

error_t ReadDegByParameter(size_t *parameter) {
    *parameter = 0;
    size_t overflow = SIZE_MAX;
    int c = getchar();
    if (!isdigit(c)) {
        IgnoreLine();
        return DEG_BY_ERROR;
    }

    while (isdigit(c)) {
        *parameter = ((*parameter) * 10) + (c - '0');
        if (*parameter > overflow) {
            IgnoreLine();
            return DEG_BY_ERROR;
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        return DEG_BY_ERROR;
    }

    return NO_ERROR;
}

// TODO -0 chyba źle obsługuję.
error_t ReadAtParameter(poly_coeff_t *parameter) {
    *parameter = 0;
    int c = getchar();
    if (c == '-') {
        *parameter = -1;
        c = getchar();
    }
    if (!isdigit(c)) {
        IgnoreLine();
        return AT_ERROR;
    }

    while (isdigit(c)) {
        *parameter = ((*parameter) * 10) + (c - '0');
        if (*parameter > LONG_MAX || *parameter < LONG_MIN) {
            IgnoreLine();
            return AT_ERROR;
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        return AT_ERROR;
    }

    return NO_ERROR;
}

error_t CheckIfDegByOrAt(char *word, char *command) {
    if (strcmp(word, "DEB_BY") == 0) {
        *command = *word;
        return NO_ERROR;
    } else if (strcmp(word, "AT") == 0) {
        *command = *word;
        return NO_ERROR;
    } else {
        return INVALID_VALUE;
    }
}

error_t ReadCommand(Command *command) {
    errno = 0;
    scanf("%s", command->name);
    if (errno != 0) {
        IgnoreLine();
        return INVALID_VALUE;
    } else {
        printf("wczytane słowo: %s\n", command->name);
        int c = getchar();
        if (c == ' ') {
            char commandName[10];
            error_t error = CheckIfDegByOrAt(command->name, commandName);
            if (error == NO_ERROR && strcmp(commandName, "DEG_BY") == 0) {
                return ReadDegByParameter(&command->degByParameter);
            } else if (error == NO_ERROR && strcmp(commandName, "AT") == 0) {
                return ReadAtParameter(&command->atParameter);
            } else {
                IgnoreLine();
                return INVALID_VALUE;
            }
        }
        if (c == '\n' || c == EOF) {
            return NO_ERROR;
        }
    }
    IgnoreLine();
    return INVALID_VALUE;
}

error_t ReadOneLineOfInput(union ParsedLine *line) {
    error_t error;
    Poly p;
    int c = getchar();
    switch (c) {
        case '#':
            IgnoreLine();
            return LINE_IGNORED;
        case '(':
            error = ReadPoly(&p);
            line->lineType = POLY;
            line->poly = p;
            return error;
        case EOF:
            return ENCOUNTERED_EOF;
        case '\n':
            return LINE_IGNORED;
        default:
            if (isdigit(c)) {
                ungetc(c, stdin);
                error = ReadConstPoly(&p, false);
                line->lineType = POLY;
                line->poly = p;
                return error;
            } else if (c == '-') {
                error = ReadConstPoly(&p, true);
                line->lineType = POLY;
                line->poly = p;
                return error;
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                Command command;
                error = ReadCommand(&command);
                line->lineType = COMMAND;
                line->command = command;
                return error;
            } else {
                return INVALID_VALUE;
            }
    }
}