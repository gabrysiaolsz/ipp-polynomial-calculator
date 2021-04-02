#ifndef PARSING_H
#define PARSING_H

#include <stdbool.h>

// Union that covers all the possible data types needed to hold a line of input.
union word_union {
    long double floating_point_number;
    long long negative_number;
    unsigned long long non_negative_number;
    char *text;
};

// Enumeration type saying which data type is stored in a union.
enum enum_word_type { NEGATIVE_NUMBER, NON_NEGATIVE_NUMBER, FLOATING_POINT_NUMBER, TEXT };

// A struct that holds the data union and a enumeration type saying which type is in the union.
typedef struct word_type {
    union word_union w_union;
    enum enum_word_type type;
} word_t;

void read_text(char *, word_t *);
bool is_character_in_base(char, const int);
bool are_characters_valid_in_given_base(const char *, int);
bool is_only_whitespaces(const char *);
bool is_hexadecimal_number(const char *);
bool is_valid_octal_number(const char *);
bool is_valid_decimal_number(const char *);
bool is_double_hexadecimal_number(const char *);
void parse_hexadecimal_number(char *, word_t *, char *);
void parse_octal_number(char *, word_t *, char *);
void parse_decimal_number(char *, word_t *, char *);
void parse_double(char *, word_t *, char *);
void parse_one_word(char *, word_t *);

#endif // PARSING_H