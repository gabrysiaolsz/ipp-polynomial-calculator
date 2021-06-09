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
/**
 * Największy możliwy rozmiar polecenia.
 */
#define MAX_COMMAND_SIZE 10

/**
 * Struktura przechowująca typ polecenia.
 */
typedef struct {
    /**
     * Tablica znaków przechowująca nazwę polecenia.
     */
    char name[MAX_COMMAND_SIZE];
    /**
     * Unia przechowująca argument polecenia DEG_BY albo AT.
     */
    union {
        size_t degByParameter;
        poly_coeff_t atParameter;
    };
} Command;

/**
 * Struktura przechowująca typ linii.
 */
typedef struct {
    /**
     *  Bool informujący o tym, czy linijka jest wielomianem, czy nie. Jeśli nie jest, to jest
     *  to polecenie.
     */
    bool isPoly;
    /**
     * Typ polecenia.
     */
    Command command;
    /**
     * Typ wielomianu.
     */
    Poly poly;
} ParsedLine;

#endif // POLYNOMIALS_CALC_H
