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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void IgnoreLine(int c) {
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

error_t ReadUnsignedCoeff(unsigned long *result) {
    *result = 0;
    unsigned long longOverflow = (unsigned long)LONG_MAX + 1;
    int c = getchar();
    if (!isdigit(c)) {
        IgnoreLine(c);
        return INVALID_VALUE;
    }

    while (isdigit(c)) {
        *result = ((*result) * 10) + (unsigned)(c - '0');
        if (*result > longOverflow) {
            IgnoreLine(c);
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
        *result = ((*result) * 10) + (unsigned)(c - '0');
        if (*result > INT_MAX) {
            IgnoreLine(c);
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

error_t ReadConstPoly(Poly *result, bool isNegative, bool isMono);

error_t ReadMono(Mono *result) {
    Poly p = PolyZero();
    Mono m;
    int c = getchar();
    error_t error;

    if (c == '(') {
        error = ReadMono(&m);
        if (error != NO_ERROR) {
            //PolyDestroy(&p);
            return error;
        }
        p = (Poly) {.size = 1, .arr = SafeMalloc(sizeof(Mono))};
        p.arr[0] = m;
    } else if (c == EOF) {
        return ENCOUNTERED_EOF;
    } else if (c == '-') {
        error = ReadConstPoly(&p, true, true);
        if (error != NO_ERROR) {
            return error;
        }
    } else {
        ungetc(c, stdin);
        error = ReadConstPoly(&p, false, true);
        if (error != NO_ERROR) {
            return error;
        }
    }

    c = getchar();
    if (c != ',') {
        IgnoreLine(c);
        return INVALID_VALUE;
    }

    unsigned int exp;
    error = ReadExp(&exp);
    if (error != NO_ERROR) {
        return error;
    }

    c = getchar();
    if (c != ')') {
        IgnoreLine(c);
        return INVALID_VALUE;
    }

    *result = MonoFromPoly(&p, (int)exp);
    if (RecursiveMonoIsZero(result)) {
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
        c = getchar();
        if(c == '('){
            error = ReadMono(&tmpMono);
            if (error != NO_ERROR) {
                PolyDestroy(polyResult);
                return error;
            }
            *polyResult = AddMonoToPoly(polyResult, &tmpMono);
            c = getchar();
        }
        else{
            IgnoreLine(c);
            PolyDestroy(polyResult);
            return INVALID_VALUE;
        }
    }

    if (c != '\n' && c != EOF) {
        PolyDestroy(polyResult);
        IgnoreLine(c);
        return INVALID_VALUE;
    }

    return NO_ERROR;
}

error_t ReadConstPoly(Poly *result, bool isNegative, bool isMono) {
    unsigned long coeff;
    error_t error = ReadUnsignedCoeff(&coeff);
    if (error != NO_ERROR) {
        return error;
    }

    int c = getchar();
    if(!isMono && c != '\n' && c!= EOF){
        IgnoreLine(c);
        return INVALID_VALUE;
    }
    if(isMono && c != '\n' && c != EOF && c != ','){
        IgnoreLine(c);
        return INVALID_VALUE;
    }
    if(c == ','){
        ungetc(c, stdin);
    }

    
    if (isNegative) {
        *result = PolyFromCoeff(-1 * (long)coeff);
    } else {
        if (coeff > LONG_MAX) {
            IgnoreLine(c);
            return INVALID_VALUE;
        }
        *result = PolyFromCoeff((long)coeff);
    }

    return NO_ERROR;
}

error_t ReadDegByParameter(unsigned long *parameter) {
    *parameter = 0;
    unsigned long previous_value;
    int c = getchar();
    if (!isdigit(c)) {
        IgnoreLine(c);
        return DEG_BY_ERROR;
    }

    while (isdigit(c)) {
        previous_value = *parameter;
        *parameter = ((*parameter) * 10) + (unsigned)(c - '0');
        if (*parameter < previous_value) {
            IgnoreLine(c);
            return DEG_BY_ERROR;
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        IgnoreLine(c);
        return DEG_BY_ERROR;
    }

    return NO_ERROR;
}

// TODO -0 chyba źle obsługuję.
error_t ReadAtParameter(poly_coeff_t *parameter) {
    unsigned long tmp = 0;
    bool isNegative;
    int c = getchar();
    if (c == '-') {
        isNegative = true;
        c = getchar();
    }
    if (!isdigit(c)) {
        IgnoreLine(c);
        return AT_ERROR;
    }

    while (isdigit(c)) {
        tmp = (tmp * 10) + (c - '0');
        if(tmp > LONG_MAX) {
            if(tmp == (unsigned)LONG_MAX + 1 && isNegative){
                *parameter = -1 * (signed)tmp;
                return NO_ERROR;
            }
            IgnoreLine(c);
            return AT_ERROR;
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        IgnoreLine(c);
        return AT_ERROR;
    }
    
    *parameter = (signed)tmp;
    
    if(isNegative){
        *parameter *= -1;
    }
    
    return NO_ERROR;
}

error_t CheckIfDegByOrAt(char *word) {
    if (strcmp(word, "DEG_BY") == 0) {
        return NO_ERROR;
    } else if (strcmp(word, "AT") == 0) {
        return NO_ERROR;
    } else {
        return INVALID_VALUE;
    }
}

error_t ReadWord(Command *command){
    int c = getchar();
    unsigned int i = 0;

    while(!isspace(c) && i < 9){
        command->name[i] = (char)c;
        i++;
        c = getchar();
    }
    command->name[i] = '\0';

    if(c != '\n' && c!= EOF){
        if(c == ' '){
            ungetc(c, stdin);
            return NO_ERROR;
        }
        else{
            if(strcmp(command->name, "DEG_BY") == 0){
                return DEG_BY_ERROR;
            }
            else if(strcmp(command->name, "AT") == 0){
                return AT_ERROR;
            }
            return INVALID_VALUE;
        }
    }
    ungetc(c, stdin);
    return NO_ERROR;
}

error_t ReadCommand(Command *command) {
    error_t error = ReadWord(command);
    if (error != NO_ERROR) {
        IgnoreLine(0);
        return error;
    }
    else {
        int c = getchar();
        error = CheckIfDegByOrAt(command->name);
        if (c == ' ') {
            if (error == NO_ERROR && strcmp(command->name, "DEG_BY") == 0) {
                return ReadDegByParameter(&command->degByParameter);
            } else if (error == NO_ERROR && strcmp(command->name, "AT") == 0) {
                return ReadAtParameter(&command->atParameter);
            } else {
                IgnoreLine(c);
                return INVALID_VALUE;
            }
        }
        else if(CheckIfDegByOrAt(command->name) == NO_ERROR){
            if(strcmp(command->name, "DEG_BY") == 0){
                return DEG_BY_ERROR;
            }
            else{
                return AT_ERROR;
            }
        }        
        if (c == '\n' || c == EOF) {
            return NO_ERROR;
        }
    }
    IgnoreLine(0);
    return INVALID_VALUE;
}

error_t ReadOneLineOfInput(ParsedLine *line) {
    error_t error;
    Poly p;
    int c = getchar();
    switch (c) {
        case '#':
            IgnoreLine(c);
            return LINE_IGNORED;
        case '(':
            error = ReadPoly(&p);
            line->isPoly = true;
            line->poly = p;
            return error;
        case EOF:
            return ENCOUNTERED_EOF;
        case '\n':
            return LINE_IGNORED;
        default:
            if (isdigit(c)) {
                ungetc(c, stdin);
                error = ReadConstPoly(&p, false, false);
                line->isPoly = true;
                line->poly = p;
                return error;
            } else if (c == '-') {
                error = ReadConstPoly(&p, true, false);
                line->isPoly = true;
                line->poly = p;
                return error;
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                Command command;
                error = ReadCommand(&command);
                line->isPoly = false;
                line->command = command;
                return error;
            } else {
                IgnoreLine(c);
                return INVALID_VALUE;
            }
    }
}