/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "errors.h"
#include "input_parser.h"
#include <stdio.h>

int main() {
    error_t error = ReadOneLineOfInput();
    printf("error to %d\n", error);

    while (error != ENCOUNTERED_EOF) {
        error = ReadOneLineOfInput();
        printf("error to %d\n", error);
    }
}