/** @file
  Interfejs kalkulatora rzadkich wielu zmiennych
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef POLYNOMIALS_CALC_H
#define POLYNOMIALS_CALC_H

#include "errors.h"
#include "poly.h"

typedef enum {
    ZERO,
    IS_COEFF,
    IS_ZERO,
    CLONE,
    ADD,
    MUL,
    NEG,
    SUB,
    IS_EQ,
    DEG,
    DEG_BY,
    AT,
    PRINT,
    POP
} CommandName;

typedef enum{
    COMMAND,
    POLY
}lineType;

typedef struct{
    char *name;
    union{
        // te dwa typy lol
    };
}Command;

union ParsedLine{
    lineType line_t;
    Command command;
    Poly poly;
};

#endif // POLYNOMIALS_CALC_H
