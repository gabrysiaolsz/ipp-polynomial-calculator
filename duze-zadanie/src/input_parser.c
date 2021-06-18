/** @file
 * Implementacja interfejsu parsera wejścia wczytującego wielomiany i polecenia.
 *
 * @author Gabriela Olszewska <go418326@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 05.2021
 */

#include "calc.h"
#include "errors.h"
#include "safe_memory_allocation.h"
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * Przechodzi linię do końca, ignorując jej zawartość.
 * @param c : wczytany znak, od którego mamy zignorować linijkę.
 */
void IgnoreLine(int c) {
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}

/**
 * Ignoruje linijkę i zwraca podany błąd.
 * @param c : znak, od którego ignorujemy linię,
 * @param error : kod błędu do zwrócenia,
 * @return : kod błędu.
 */
error_t IgnoreLineAndReturnError(int c, error_t error) {
    IgnoreLine(c);
    return error;
}

/**
 * Wczytuje nieujemny współczynnik wielomianu.
 * @param *result : wskaźnik do zapisania wyniku,
 * @return : kod błędu.
 */
error_t ReadUnsignedCoeff(unsigned long *result) {
    *result = 0;
    unsigned long longOverflow = (unsigned long)LONG_MAX + 1;
    int c = getchar();
    if (!isdigit(c)) {
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }

    while (isdigit(c)) {
        *result = ((*result) * 10) + (unsigned)(c - '0');
        if (*result > longOverflow) {
            return IgnoreLineAndReturnError(c, INVALID_VALUE);
        }
        c = getchar();
    }

    if (c != EOF) {
        ungetc(c, stdin);
    }

    return NO_ERROR;
}

/**
 * Wczytuje nieujemny wykładnik wielomianu.
 * @param *result : wskaźnik do zapisania wyniku,
 * @return : kod błędu.
 */
error_t ReadExp(unsigned int *result) {
    *result = 0;
    int c = getchar();
    while (isdigit(c)) {
        *result = ((*result) * 10) + (unsigned)(c - '0');
        if (*result > INT_MAX) {
            return IgnoreLineAndReturnError(c, INVALID_VALUE);
        }
        c = getchar();
    }

    if (c != EOF) {
        ungetc(c, stdin);
    }

    return NO_ERROR;
}

/**
 * Wczytuje wielomian stały.
 * @param *result : wskaźnik na zapisanie wielomianu wynikowego.
 * @param isNegative : informacja o tym, czy wczytywany wielomian jest ujemny.
 * @param isMonosCoeff : informacja o tym, czy wczytywany wielomian to współczynnik jednomianu.
 * @return : kod błędu.
 */
error_t ReadConstPoly(Poly *result, bool isNegative, bool isMonosCoeff) {
    unsigned long coeff;
    error_t error = ReadUnsignedCoeff(&coeff);
    if (error != NO_ERROR) {
        return error;
    }

    int c = getchar();
    if (!isMonosCoeff && c != '\n' && c != EOF) {
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }
    if (isMonosCoeff && c != EOF && c != ',') {
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }
    if (c == ',') {
        ungetc(c, stdin);
    }

    if (isNegative) {
        *result = PolyFromCoeff(-1 * (poly_coeff_t)coeff);
    } else {
        if (coeff > LONG_MAX) {
            return IgnoreLineAndReturnError(c, INVALID_VALUE);
        }
        *result = PolyFromCoeff((poly_coeff_t)coeff);
    }

    return NO_ERROR;
}

error_t ReadPoly(Poly *polyResult, bool requireEOL);

/**
 * Wczytuje współczynnik jednomianu, zapisuje go na wielomianie.
 * @param *p : wskaźnik na zapisanie wyniku.
 * @return kod błędu.
 */
error_t ReadMonosCoeff(Poly *p) {
    *p = PolyZero();
    int c = getchar();
    error_t error;

    if (c == EOF) {
        return ENCOUNTERED_EOF;
    }

    if (c == '(') {
        error = ReadPoly(p, false);
    } else if (c == '-') {
        error = ReadConstPoly(p, true, true);
    } else {
        ungetc(c, stdin);
        error = ReadConstPoly(p, false, true);
    }
    if (error != NO_ERROR) {
        return error;
    }
    if ((c = getchar()) != ',') {
        PolyDestroy(p);
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }

    return error;
}

/**
 * Wczytuje jednomian.
 * @param *result : wskaźnik do zapisania wyniku.
 * @return : kod błędu.
 */
error_t ReadMono(Mono *result) {
    int c;
    Poly p;

    error_t error = ReadMonosCoeff(&p);
    if (error != NO_ERROR) {
        return error;
    }

    unsigned int exp;
    error = ReadExp(&exp);
    if (error != NO_ERROR) {
        return error;
    }

    if ((c = getchar()) != ')') {
        PolyDestroy(&p);
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }

    *result = MonoFromPoly(&p, (poly_exp_t)exp);
    if (RecursiveMonoIsZero(result)) {
        PolyDestroy(&result->p);
        *result = (Mono){.exp = (int)exp, .p = PolyZero()};
    }

    return NO_ERROR;
}

/**
 * Dodaje jednomian do wielomianu.
 * @param p : wielomian do którego dodajemy jednomian,
 * @param m : jednomian, który dodajemy.
 * @return wielomian będący wynikiem zsumowania argumentów.
 */
Poly AddMonoToPoly(Poly *p, Mono *m) {
    if (PolyIsZero(&m->p)) {
        return *p;
    }
    if (PolyIsZero(p)) {
        poly_coeff_t tmp;
        if (m->exp == 0 && RecursivePolyIsCoeff(&m->p, &tmp)) {
            return PolyFromCoeff(tmp);
        } else {
            Poly polyResult = {.size = 1, .arr = SafeMalloc(sizeof(Mono))};
            polyResult.arr[0] = *m;
            return polyResult;
        }
    }

    Poly tmpPoly = {.size = 1, .arr = SafeMalloc(sizeof(Mono))};
    tmpPoly.arr[0] = *m;
    Poly result = PolyAdd(p, &tmpPoly);
    PolyDestroy(&tmpPoly);
    return result;
}

/**
 * Wczytuje wielomian.
 * @param *polyResult : wskaźnik do zapisania wielomianu wynikowego,
 * @param requireEOL : informacja, czy powinniśmy oczekiwać znaku końca linii bądź pliku.
 * @return kod błędu.
 */
error_t ReadPoly(Poly *polyResult, bool requireEOL) {
    *polyResult = PolyZero();
    Mono tmpMono;
    int c;

    while (true) {
        error_t error = ReadMono(&tmpMono);
        if (error != NO_ERROR) {
            PolyDestroy(polyResult);
            return error;
        }
        Poly tmpPoly = *polyResult;
        *polyResult = AddMonoToPoly(&tmpPoly, &tmpMono);
        PolyDestroy(&tmpPoly);

        if ((c = getchar()) != '+') {
            break;
        }

        if ((c = getchar()) != '(') {
            PolyDestroy(polyResult);
            return IgnoreLineAndReturnError(c, INVALID_VALUE);
        }
    }

    if (requireEOL && c != '\n' && c != EOF) {
        PolyDestroy(polyResult);
        return IgnoreLineAndReturnError(c, INVALID_VALUE);
    }

    if (!requireEOL) {
        ungetc(c, stdin);
    }

    return NO_ERROR;
}

/**
 * Wczytuje parametr polecenia DEG_BY
 * @param *parameter : wskaźnik na zapisanie parametru,
 * @return kod błędu.
 */
error_t ReadDegByParameter(unsigned long *parameter) {
    *parameter = 0;
    unsigned long previous_value;
    int c = getchar();

    if (!isdigit(c)) {
        return IgnoreLineAndReturnError(c, DEG_BY_ERROR);
    }

    while (isdigit(c)) {
        previous_value = *parameter;
        *parameter = ((*parameter) * 10) + (unsigned)(c - '0');
        if (*parameter < previous_value) {
            return IgnoreLineAndReturnError(c, DEG_BY_ERROR);
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        return IgnoreLineAndReturnError(c, DEG_BY_ERROR);
    }

    return NO_ERROR;
}

/**
 * Wczytuje parametr polecenia AT.
 * @param *parameter : wskaźnik na zapisanie parametru,
 * @return : kod błędu.
 */
error_t ReadAtParameter(poly_coeff_t *parameter) {
    unsigned long tmp = 0;
    bool isNegative = false;
    int c = getchar();

    if (c == '-') {
        isNegative = true;
        c = getchar();
    }
    if (!isdigit(c)) {
        return IgnoreLineAndReturnError(c, AT_ERROR);
    }

    while (isdigit(c)) {
        tmp = (tmp * 10) + ((unsigned)c - '0');
        if (tmp > LONG_MAX) {
            if (tmp - 1 == LONG_MAX && isNegative) {
                *parameter = -1 * (signed)tmp;
                return NO_ERROR;
            }
            return IgnoreLineAndReturnError(c, AT_ERROR);
        }
        c = getchar();
    }

    if (c != EOF && c != '\n') {
        return IgnoreLineAndReturnError(c, AT_ERROR);
    }

    *parameter = (signed)tmp;

    if (isNegative) {
        if (*parameter == 0) {
            return AT_ERROR;
        }
        *parameter *= -1;
    }

    return NO_ERROR;
}

/**
 * Wczytuje słowo.
 * @param *command : wskaźnik na polecenie, na którym zapisujemy słowo.
 * @return kod błędu.
 */
error_t ReadWord(Command *command) {
    int c = getchar();
    unsigned int i = 0;

    while (!isspace(c) && (c != '\0') && (c != EOF) && i < MAX_COMMAND_SIZE - 1) {
        command->name[i] = (char)c;
        i++;
        c = getchar();
    }
    command->name[i] = '\0';

    if (c != '\n' && c != EOF) {
        if (c == ' ') {
            ungetc(c, stdin);
            return NO_ERROR;
        } else {
            if (strcmp(command->name, "DEG_BY") == 0) {
                return DEG_BY_ERROR;
            } else if (strcmp(command->name, "AT") == 0) {
                return AT_ERROR;
            }
            return INVALID_VALUE;
        }
    }
    if (c != EOF) {
        ungetc(c, stdin);
    }

    return NO_ERROR;
}

/**
 * Wczytuje polecenie.
 * @param *command : wskaźnik na polecenie, na którym zapisujemy wczytane wartości.
 * @return : kod błędu.
 */
error_t ReadCommand(Command *command) {
    error_t error = ReadWord(command);

    if (error != NO_ERROR) {
        return IgnoreLineAndReturnError(0, error);
    } else {
        int c = getchar();
        if (c == ' ') {
            if (strcmp(command->name, "DEG_BY") == 0) {
                return ReadDegByParameter(&command->degByParameter);
            } else if (strcmp(command->name, "AT") == 0) {
                return ReadAtParameter(&command->atParameter);
            } else {
                return IgnoreLineAndReturnError(c, INVALID_VALUE);
            }
        }
        if (strcmp(command->name, "DEG_BY") == 0) {
            return DEG_BY_ERROR;
        }
        if (strcmp(command->name, "AT") == 0) {
            return AT_ERROR;
        }
        if (c == '\n' || c == EOF) {
            return NO_ERROR;
        }
    }

    return IgnoreLineAndReturnError(0, INVALID_VALUE);
}

/**
 * Wczytuje jedną linijkę wejścia.
 * @param *line : wskaźnik na typ linii, na którym zapisujemy wyniki.
 * @return : kod błędu.
 */
error_t ReadOneLineOfInput(ParsedLine *line) {
    line->isPoly = true;
    int c = getchar();
    switch (c) {
        case '#':
            return IgnoreLineAndReturnError(c, LINE_IGNORED);
        case '(':
            return ReadPoly(&line->poly, true);
        case EOF:
            return ENCOUNTERED_EOF;
        case '\n':
            return LINE_IGNORED;
        default:
            if (isdigit(c)) {
                ungetc(c, stdin);
                return ReadConstPoly(&line->poly, false, false);
            } else if (c == '-') {
                return ReadConstPoly(&line->poly, true, false);
            } else if (isalpha(c)) {
                ungetc(c, stdin);
                line->isPoly = false;
                return ReadCommand(&line->command);
            } else {
                return IgnoreLineAndReturnError(c, INVALID_VALUE);
            }
    }
}

//TODO poprawić wczytywanie COMPOSE