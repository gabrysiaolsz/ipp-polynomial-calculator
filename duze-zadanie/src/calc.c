/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "errors.h"
#include "input_parser.h"
#include "stack.h"
#include <stdio.h>
#include <string.h>

void ExecuteZero(Stack *stack){

}

void ExecuteCommand(Command command, Stack *stack){
    if(strcmp(command.name, "ZERO") == 0){
        ExecuteZero(stack);
    }
    else if(strcmp(command.name, "IS_COEFF") == 0){
        ;
    }
    else if(strcmp(command.name, "IS_ZERO") == 0){
        ;
    }
    else if(strcmp(command.name, "CLONE") == 0){
        ;
    }
    else if(strcmp(command.name, "ADD") == 0){
        ;
    }
    else if(strcmp(command.name, "MUL") == 0){
        ;
    }
    else if(strcmp(command.name, "NEG") == 0){
        ;
    }
    else if(strcmp(command.name, "SUB") == 0){
        ;
    }
    else if(strcmp(command.name, "IS_EQ") == 0){
        ;
    }
    else if(strcmp(command.name, "DEG") == 0){
        ;
    }
    else if(strcmp(command.name, "DEB_BY") == 0){
        ;
    }
    else if(strcmp(command.name, "AT") == 0){
        ;
    }
    else if(strcmp(command.name, "PRINT") == 0){
        ;
    }
    else if(strcmp(command.name, "POP") == 0){
        ;
    }
}

void PushPoly(Stack *stack, Poly p){
    Push(stack, p);
}

void ExecuteInput(Stack *stack){
    int lineNumber = 1;
    union ParsedLine line;
    while(true){
        error_t error = ReadOneLineOfInput(&line);
        switch (error) {
            case NO_ERROR:
                if(line.line_t == POLY){
                    PushPoly(stack, line.poly);
                }
                else if(line.line_t == COMMAND){
                    ExecuteCommand(line.command);
                }
                break;
            case INVALID_VALUE:
                printf("invalid value on line %d\n", lineNumber);
                break;
            case LINE_IGNORED:
                printf("ignored line %d\n", lineNumber);
                break;
            case DEG_BY_ERROR:
                printf("invalid deg_by command on line %d\n", lineNumber);
                break;
            case AT_ERROR:
                printf("invalid at command on line %d\n", lineNumber);
                break;
            case ENCOUNTERED_EOF:
                return;
            default:
                ;
        }
        lineNumber++;
    }
}

int main() {
    Stack *stack = CreateStack();
    ExecuteInput(stack);
}