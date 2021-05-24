/** @file
 * Plik zawierający typ reprezentujący różne błędy.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#ifndef POLYNOMIALS_ERRORS_H
#define POLYNOMIALS_ERRORS_H

/**
 * Typ wyliczeniowy przechowujący informacje o możliwych błędach, zwracanych w trakcie trwania
 * programu.
 */
typedef enum {
    NO_ERROR, // Brak błędu.
    INVALID_VALUE, // Niepoprawna wartość.
    LINE_IGNORED, // Linia zignorowana.
    ENCOUNTERED_EOF, // Natrafiliśmy na koniec pliku.
    DEG_BY_ERROR, // Błąd przy wczytywaniu polecenia DEG_BY.
    AT_ERROR, // Błąd przy wczytywaniu polecenia AT.
} error_t;

#endif // POLYNOMIALS_ERRORS_H
