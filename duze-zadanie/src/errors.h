/** @file
 * Plik zawierający typ reprezentujący błąd.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef POLYNOMIALS_ERRORS_H
#define POLYNOMIALS_ERRORS_H

typedef enum {
    NO_ERROR,
    INVALID_VALUE,
    LINE_IGNORED,
    ENCOUNTERED_EOF,
    DEG_BY_ERROR,
    AT_ERROR,
} error_t;

#endif // POLYNOMIALS_ERRORS_H
