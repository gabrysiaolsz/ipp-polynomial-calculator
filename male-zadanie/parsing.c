#include "parsing.h"
#include "safe_memory_allocation.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#define ZERO_ASCII '0'
#define HEXADECIMAL_PREFIX "0x"
#define VALID_HEXADECIMAL_CHARS "0123456789abcdef"
#define INVALID_HEXADECIMAL_CHARS "0123456789abcdefp."
#define VALID_OCTAL_CHARS "01234567"
#define VALID_DECIMAL_CHARS "0123456789"
#define HEXADECIMAL_BASE 16
#define DECIMAL_BASE 10
#define OCTAL_BASE 8
#define INVALID_HEXADECIMAL_BASE 1
#define WHITE_BASE 0
#define WHITE_DELIMITERS "\t\v\f\r \n"

// Reads text and saves it to a given word.
void read_text(char *text, word_t *word) {
    word->w_union.text = safe_malloc((strlen(text) + 1) * sizeof(word_t));
    strcpy(word->w_union.text, text);
    word->type = TEXT;
}

// Return true when a given character is in specified base, false otherwise.
bool is_character_in_base(char character, const int base) {
    switch (base) {
        case OCTAL_BASE:
            return strchr(VALID_OCTAL_CHARS, (int)character);
        case DECIMAL_BASE:
            return strchr(VALID_DECIMAL_CHARS, (int)character);
        case HEXADECIMAL_BASE:
            return strchr(VALID_HEXADECIMAL_CHARS, (int)character);
        case WHITE_BASE:
            return strchr(WHITE_DELIMITERS, (int)character);
        case INVALID_HEXADECIMAL_BASE:
            return strchr(INVALID_HEXADECIMAL_CHARS, (int)character);
        default:
            return false;
    }
}

// Returns true when given word contains only characters valid in specified base,
// false otherwise.
bool are_characters_valid_in_given_base(const char *word, int base) {
    while (*word) {
        if (!is_character_in_base(*word, base)) {
            return false;
        }
        word++;
    }
    return true;
}

// Returns true when given word contains only whitespaces, false otherwise.
bool is_only_whitespaces(const char *word) {
    return are_characters_valid_in_given_base(word, WHITE_BASE);
}

// Returns true when given word is a hexadecimal number, false otherwise.
bool is_hexadecimal_number(const char *word) {
    if (strlen(word) < 2) {
        return false;
    }
    if (word[0] == '+' || word[0] == '-') {
        word++;
    }
    if (strncmp(word, HEXADECIMAL_PREFIX, 2) != 0) { // First two characters aren't the hex_prefix
        return false;
    }
    return are_characters_valid_in_given_base(word + 2, HEXADECIMAL_BASE);
}

// Return true when given word is a valid octal number, false otherwise.
bool is_valid_octal_number(const char *word) {
    if (*word != ZERO_ASCII) {
        return false;
    }
    return are_characters_valid_in_given_base(word, OCTAL_BASE);
}

// Returns true when given word is a valid decimal number, false otherwise.
bool is_valid_decimal_number(const char *word) {
    if (word[0] == '+' || word[0] == '-') {
        if (word[1] != '\0') {
            word++;
        } else {
            return false;
        }
    }
    return are_characters_valid_in_given_base(word, DECIMAL_BASE);
}

// Returns true when a given word is a valid hexadecimal number in floating-point format, false
// otherwise.
bool is_double_hexadecimal_number(const char *word) {
    if (strlen(word) < 2) {
        return false;
    }
    if (word[0] == '+' || word[0] == '-') {
        word++;
    }
    if (strncmp(word, HEXADECIMAL_PREFIX, 2) != 0) { // First two characters aren't the hex_prefix
        return false;
    }
    return are_characters_valid_in_given_base(word + 2, INVALID_HEXADECIMAL_BASE);
}

// Decides if given word is indeed a valid hexadecimal number or a text.
// After that, parses it and saves to a given word.
void parse_hexadecimal_number(char *input_word, word_t *parsed_word, char *end) {
    if (input_word[0] != ZERO_ASCII) {
        read_text(input_word, parsed_word);
    } else {
        errno = 0;
        unsigned long long hex_value = strtoull(input_word, &end, HEXADECIMAL_BASE);
        if (errno != 0) {
            read_text(input_word, parsed_word);
        } else {
            parsed_word->type = NON_NEGATIVE_NUMBER;
            parsed_word->w_union.non_negative_number = hex_value;
        }
    }
}

// Decides if given word is indeed a valid octal number or a text.
// After that, parses it and saves to a given word.
void parse_octal_number(char *input_word, word_t *parsed_word, char *end) {
    errno = 0;
    unsigned long long oct_value = strtoull(input_word, &end, OCTAL_BASE);
    if (errno != 0) {
        read_text(input_word, parsed_word);
    } else {
        parsed_word->type = NON_NEGATIVE_NUMBER;
        parsed_word->w_union.non_negative_number = oct_value;
    }
}

// Decides if a given word is a negative, non-negative number or a text.
// After that, parses it and saves to a given word.
void parse_decimal_number(char *input_word, word_t *parsed_word, char *end) {
    if (input_word[0] == '-') {
        errno = 0;
        long long neg_value = strtoll(input_word, &end, DECIMAL_BASE);
        if (errno != 0) {
            read_text(input_word, parsed_word);
        } else {
            if (neg_value == 0) { // Corner case: 0.
                parsed_word->type = NON_NEGATIVE_NUMBER;
                parsed_word->w_union.non_negative_number = 0;
            } else {
                parsed_word->type = NEGATIVE_NUMBER;
                parsed_word->w_union.negative_number = neg_value;
            }
        }
    } else {
        errno = 0;
        unsigned long long pos_value = strtoull(input_word, &end, DECIMAL_BASE);
        if (errno != 0) {
            read_text(input_word, parsed_word);
        } else {
            parsed_word->type = NON_NEGATIVE_NUMBER;
            parsed_word->w_union.non_negative_number = pos_value;
        }
    }
}

// Decides if a given word is a double, or a text. If it's parsed as a double but is an integer
// within the range, saves it as a negative or non-negative value.
void parse_double(char *input_word, word_t *parsed_word, char *end) {
    errno = 0;
    long double value_d = strtold(input_word, &end);
    if ((errno != 0) || (end == input_word) || (*end != '\0')) {
        read_text(input_word, parsed_word);
    } else if (value_d != value_d) {
        read_text("nan", parsed_word);
    } else if (value_d < 0) {
        if (value_d - (long long)value_d == 0 && value_d >= LLONG_MIN) {
            parsed_word->type = NEGATIVE_NUMBER;
            parsed_word->w_union.negative_number = (long long)value_d;
        } else {
            parsed_word->w_union.floating_point_number = value_d;
            parsed_word->type = FLOATING_POINT_NUMBER;
        }
    } else if (value_d >= 0) {
        if (value_d - (unsigned long long)value_d == 0 && value_d <= ULLONG_MAX) {
            parsed_word->type = NON_NEGATIVE_NUMBER;
            parsed_word->w_union.non_negative_number = (unsigned long long)value_d;
        } else {
            parsed_word->w_union.floating_point_number = value_d;
            parsed_word->type = FLOATING_POINT_NUMBER;
        }
    }
}

// Parses one given word from input and saves it to a given variable.
// While parsing, decides whether it's a: negative integer, non-negative integer, floating point
// number or a text.
void parse_one_word(char *input_word, word_t *parsed_word) {
    char *end = NULL;
    if (is_hexadecimal_number(input_word)) {
        parse_hexadecimal_number(input_word, parsed_word, end);
    } else if (is_valid_octal_number(input_word)) {
        parse_octal_number(input_word, parsed_word, end);
    } else if (is_valid_decimal_number(input_word)) {
        parse_decimal_number(input_word, parsed_word, end);
    } else if (is_double_hexadecimal_number(input_word)) {
        read_text(input_word, parsed_word);
    } else {
        parse_double(input_word, parsed_word, end);
    }
}