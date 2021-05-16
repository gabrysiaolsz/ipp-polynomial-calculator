/** @file
 * Interfejs kalkulatora działającego na wielomianach i stosujący odwrotną notację polską.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "calc.h"
#include "safe_memory_allocation.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

void IgnoreLine() {
    int c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

poly_coeff_t ReadCoeff(){
    poly_coeff_t coeff = 0;
    int c = getchar();
    while (isdigit(c)){
        coeff = (coeff * 10) + (c - '0');
        c = getchar();
    }
    ungetc(c, stdin);
    return coeff;
}

poly_exp_t ReadExp(){
    poly_exp_t exp = 0;
    int c = getchar();
    while(isdigit(c)){
        exp = (exp * 10) + (c - '0');
        c = getchar();
    }
    ungetc(c, stdin);

    return exp;
}

Mono ReadMono() {
    Poly p;

    int c = getchar();
    if(c == '('){
        Mono m = ReadMono();
        p = (Poly){.arr = SafeMalloc(sizeof(Poly)), .size = 1};
        p.arr[0] = m;
    }
    else{
        ungetc(c, stdin);
        poly_coeff_t coeff = ReadCoeff();
        p = PolyFromCoeff(coeff);
    }

    c = getchar();
    if(c != ','){
        printf("niepoprawny format\n");
    }

    poly_exp_t exp = ReadExp();
    c = getchar();
    if(c == ')'){
        printf("skonczylismy :)\n");
    }
    else{
        printf("lol błąd\n");
    }

    return MonoFromPoly(&p, exp);
}

Poly ReadPoly() {
    Poly p;
    Mono m1 = ReadMono();
    int c = getchar();
    if(c == '+'){
        printf("mamy wielomian z plusem!!!");
        getchar();
        Mono m2 = ReadMono();
        Mono *monos = SafeMalloc(2 * sizeof(Mono));
        monos[0] = m1;
        monos[1] = m2;
        p = PolyAddMonos(2, monos);
    }
    else{
        p = (Poly){.arr = SafeMalloc(sizeof(Poly)), .size = 1};
        p.arr[0] = m1;
    }

    return p;
}

Poly ReadConstPoly(bool isNegative) {
    poly_coeff_t coeff = ReadCoeff();
    if(isNegative){
        return PolyFromCoeff(-1 * coeff);
    }
    else{
        return PolyFromCoeff(coeff);
    }
}

void ReadOrder() {
    errno = 0;
    char word[30];
    scanf("%s", word);
    if(errno != 0){
        printf("niepoprawne dane\n");
    }
    else{
        printf("wczytane słowo: %s\n", word);
    }
}

void ReadOneLineOfInput() {
    int c = getchar();

    switch (c) {
        case '#':
            printf("ignore line\n");
            IgnoreLine();
            break;
        case '(':
            printf("read poly\n");
            ReadPoly();
            break;
        case EOF:
            printf("end of file\n");
            break;
        case '\n':
            printf("nowa linia\n");
            return;
        default:
            if (isdigit(c)) {
                printf("read const poly +\n");
                ReadConstPoly(false);
            } else if (c == '-') {
                printf("read const poly -\n");
                ReadConstPoly(true);
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                printf("read order\n");
                ReadOrder();
            } else {
                printf("nie pasuje\n");
            }
    }
}

int main(){
    ReadOneLineOfInput();
}