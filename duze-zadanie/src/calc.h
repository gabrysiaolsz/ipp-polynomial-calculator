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

typedef enum { COMMAND, POLY } lineCommandType;

typedef struct {
    char *name;
    union {
        size_t degByParameter;
        poly_coeff_t atParameter;
    };
} Command;

union ParsedLine {
    lineCommandType lineType;
    Command command;
    Poly poly;
};

#endif // POLYNOMIALS_CALC_H
