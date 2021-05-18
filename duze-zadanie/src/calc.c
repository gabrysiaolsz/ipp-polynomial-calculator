/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */


#include "input_parser.h"
#include "errors.h"
#include <stdio.h>

int main() {
    inputValue_t error = ReadOneLineOfInput();
    printf("error to %d\n", error);

    while (error != ENCOUNTERED_EOF){
        error = ReadOneLineOfInput();
        printf("error to %d\n", error);
    }
}