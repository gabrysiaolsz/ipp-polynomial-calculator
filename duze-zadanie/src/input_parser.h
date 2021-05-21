/** @file
 * Interfejs parsera wejścia wczytującego wielomiany i polecenia.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef POLYNOMIALS_INPUT_PARSER_H
#define POLYNOMIALS_INPUT_PARSER_H

#include "calc.h"

error_t ReadOneLineOfInput(ParsedLine *line);

#endif // POLYNOMIALS_INPUT_PARSER_H
