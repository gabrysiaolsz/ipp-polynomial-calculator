#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTING_SIZE 5
#define WHITE_DELIMITERS "\t\v\f\r \n"
#define FIRST_VALID_ASCII 33
#define LAST_VALID_ASCII 126
#define X_ASCII 'x'
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

// A struct that holds all the information about one line - a pointer to an array containing
// all the words, number of the words, whether it's a comment or there's an error
// and a number of line.
typedef struct line_type {
    word_t *word_array;
    size_t words_count;
    size_t line_number;
    bool is_comment;
    bool is_error;
    bool is_empty;
} line_t;

// Checks whether malloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what malloc has returned.
void *safe_malloc(size_t size) {
    void *malloc_ptr = malloc(size);
    if (malloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return malloc_ptr;
}

// Checks whether realloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what realloc has returned.
void *safe_realloc(void *ptr, size_t size) {
    void *realloc_ptr = realloc(ptr, size);
    if (realloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return realloc_ptr;
}

// Checks whether calloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what calloc has returned.
void *safe_calloc(size_t number_of_elements, size_t size) {
    void *calloc_ptr = calloc(number_of_elements, size);
    if (calloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return calloc_ptr;
}

// Returns true when the given character is valid (is within ASCII 33-126 or whitespace)
// false otherwise.
bool is_valid_character(int character) {
    return ((character >= FIRST_VALID_ASCII && character <= LAST_VALID_ASCII) ||
            isspace(character));
}

// Returns true if given line is valid, false otherwise.
bool is_line_valid(char *line, ssize_t size) {
    for (ssize_t i = 0; i < size; i++) {
        if (!is_valid_character(line[i])) {
            return false;
        }
    }
    return true;
}

// Changes all alphabetic characters from a line of input to lowercase.
void change_to_lower_case(char *line) {
    for (unsigned int i = 0; i < strlen(line); i++) {
        if (isalpha(line[i])) {
            line[i] = (char)tolower(line[i]);
        }
    }
}

// Gets one line from input, returns the pointer to the array containing characters from the
// input and saves the size on a given variable.
char *get_line_from_input(ssize_t *size) {
    size_t buff_size = STARTING_SIZE;

    char *buffer = safe_malloc(buff_size * sizeof(char));

    errno = 0;
    *size = getline(&buffer, &buff_size, stdin);
    if (*size == -1) {
        free(buffer);
        if (errno != 0) {
            printf("%s", strerror(errno));
            perror("Get line didn't work");
            exit(EXIT_FAILURE);
        } else {
            return NULL;
        }
    }

    return buffer;
}

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

// Returns a pointer to an array containing all words from a given input line.
// Sets the given variable to number of words in the line.
word_t *get_word_array(char *input_line, size_t *number_of_elements) {
    size_t size = 0, capacity = STARTING_SIZE;
    word_t *array = safe_malloc(capacity * sizeof(word_t));
    word_t parsed_word;
    char *word_from_line = strtok(input_line, WHITE_DELIMITERS);

    while (word_from_line != NULL) {
        if (size == capacity) {
            capacity *= 2;
            array = safe_realloc(array, (capacity * sizeof(word_t)));
        }

        parse_one_word(word_from_line, &parsed_word);
        array[size] = parsed_word;
        size++;
        word_from_line = strtok(NULL, WHITE_DELIMITERS);
    }
    *number_of_elements = size;

    return array;
}

// Checks whether a line is a comment or valid and sets corresponding boolean variables
// accordingly.
// If it is a valid, non-comment line, fills the array with words from a given line of input.
line_t make_line(char *input_line, size_t number_of_line, ssize_t size) {
    line_t line = {0};
    line.line_number = number_of_line;

    change_to_lower_case(input_line);

    if (input_line[0] == '#') {
        line.is_comment = true;
    }
    if (!is_line_valid(input_line, size) && !line.is_comment) {
        line.is_error = true;
        fprintf(stderr, "%s %zu\n", "ERROR", line.line_number);
    }
    if (is_only_whitespaces(input_line)) {
        line.is_empty = true;
    }
    if (!line.is_comment && !line.is_error && !line.is_empty) {
        size_t number_of_elements = 0;
        line.word_array = get_word_array(input_line, &number_of_elements);
        line.words_count = number_of_elements;
    }
    return line;
}

// Compares given non-negative integers numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_non_negatives(const void *a, const void *b) {
    unsigned int first = *(const unsigned int *)a;
    unsigned int second = *(const unsigned int *)b;
    return (first > second) - (first < second);
}

// Compares two given words and returns -1/0/1 when the first one is smaller/equal/bigger than the
// second one.
// The comparison is firstly done by a type of word, and when they're the same by the value.
int compare_two_words(const void *p, const void *q) {
    word_t word1 = *(const word_t *)p;
    word_t word2 = *(const word_t *)q;

    if (word1.type != word2.type) {
        return (word1.type > word2.type) - (word1.type < word2.type);
    }
    switch (word1.type) {
        case NEGATIVE_NUMBER:
            return (word1.w_union.negative_number > word2.w_union.negative_number) -
                   (word1.w_union.negative_number < word2.w_union.negative_number);
        case NON_NEGATIVE_NUMBER:
            return (word1.w_union.non_negative_number > word2.w_union.non_negative_number) -
                   (word1.w_union.non_negative_number < word2.w_union.non_negative_number);
        case FLOATING_POINT_NUMBER:
            return (word1.w_union.floating_point_number < word2.w_union.floating_point_number) -
                   (word1.w_union.floating_point_number > word2.w_union.floating_point_number);
        case TEXT:
            return strcmp(word1.w_union.text, word2.w_union.text);
        default:
            return 0;
    }
}

// Sorts the elements of one line.
void sort_one_line(line_t line) {
    qsort(line.word_array, line.words_count, sizeof(word_t), compare_two_words);
}

// Compares two given lines and returns -1/0/1 when the first one is smaller/equal/bigger than the
// second one.
// The comparison is firstly made by the number of words in lines, and when they're the same by
// the words.
int compare_two_lines(const void *p, const void *q) {
    line_t line1 = *(const line_t *)p;
    line_t line2 = *(const line_t *)q;

    if (line1.words_count == line2.words_count) {
        for (unsigned int i = 0; i < line1.words_count; i++) {
            const void *word1_ptr = &line1.word_array[i];
            const void *word2_ptr = &line2.word_array[i];
            int result = compare_two_words(word1_ptr, word2_ptr);

            if (result != 0) {
                return result;
            }
        }
        return 0;
    } else {
        return (line1.words_count > line2.words_count) - (line1.words_count < line2.words_count);
    }
}

// Sorts all the lines.
void sort_all_lines(line_t *line_array, unsigned int size) {
    qsort(line_array, size, sizeof(line_t), compare_two_lines);
}

// Frees memory allocated for all the lines.
void free_line_array(line_t *line_array, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        for (unsigned int j = 0; j < line_array[i].words_count; j++) {
            if (line_array[i].word_array[j].type == TEXT) {
                free(line_array[i].word_array[j].w_union.text);
            }
        }
        free(line_array[i].word_array);
    }
    free(line_array);
}

// Returns number of sets made from similar lines stored a given array.
size_t get_number_of_sets(line_t *line_array, unsigned int size) {
    unsigned int number = 0;
    for (unsigned int i = 0; i < size - 1; i++) {
        const void *line1_ptr = &line_array[i];
        const void *line2_ptr = &line_array[i + 1];

        if (compare_two_lines(line1_ptr, line2_ptr)) {
            number++;
        }
    }
    return number + 1;
}

// Returns set cardinality.
size_t get_set_cardinality(line_t *line_array, size_t set_start_index, unsigned int size) {
    size_t size_of_set = 1;
    bool is_still_the_same_set = true;
    while (is_still_the_same_set && set_start_index < size - 1) {
        const void *line1_ptr = &line_array[set_start_index];
        const void *line2_ptr = &line_array[set_start_index + 1];
        int res = compare_two_lines(line1_ptr, line2_ptr);
        if (res == 0) {
            size_of_set++;
        } else {
            is_still_the_same_set = false;
        }
        set_start_index++;
    }
    return size_of_set;
}

// Sorts the set array.
void sort_set_array(size_t *set_array, size_t size) {
    qsort(set_array, size, sizeof(size_t), compare_non_negatives);
}

// Frees the result array.
void free_result_array(size_t **result_array, size_t number_of_sets) {
    for (unsigned int i = 0; i < number_of_sets; i++) {
        free(result_array[i]);
    }
    free(result_array);
}

// Prints the array.
void print_array(size_t *array, size_t size) {
    printf("%zu", array[0]);
    for (size_t k = 1; k < size; k++) {
        printf(" %zu", array[k]);
    }
    printf("\n");
}

// Compares pointers to non-negative values. Returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_non_negative_pointers(const void *a, const void *b) {
    const unsigned int *first = *(const unsigned int **)a;
    const unsigned int *second = *(const unsigned int **)b;

    return (*first > *second) - (*first < *second);
}

// Prints the result.
void print_result(size_t number_of_sets, size_t **result_array, const size_t *set_sizes) {
    for (unsigned int k = 0; k < number_of_sets; k++) {
        size_t set_cardinality = set_sizes[result_array[k][0] - 1];
        print_array(result_array[k], set_cardinality);
    }
}

// Sorts a 2D array containing result.
void sort_result_array(size_t **result_array, size_t number_of_sets) {
    qsort(result_array, number_of_sets, sizeof(size_t *), compare_non_negative_pointers);
}

// Prepares the result and prints it.
void get_result(line_t *line_array, unsigned int line_array_size, size_t max_line_num) {
    size_t number_of_sets = get_number_of_sets(line_array, line_array_size);
    size_t set_index = 0;
    unsigned int line_index = 0;
    size_t **result_array = safe_malloc(number_of_sets * sizeof(size_t *));
    size_t *set_sizes = safe_calloc(max_line_num, sizeof(size_t));

    for (unsigned int i = 0; i < number_of_sets; i++) {
        size_t set_cardinality = get_set_cardinality(line_array, set_index, line_array_size);
        result_array[i] = safe_malloc(set_cardinality * sizeof(size_t));
        for (unsigned int j = 0; j < set_cardinality; j++) {
            result_array[i][j] = line_array[line_index].line_number;
            line_index++;
        }
        sort_set_array(result_array[i], set_cardinality);
        set_sizes[result_array[i][0] - 1] = set_cardinality;
        set_index += set_cardinality;
    }

    sort_result_array(result_array, number_of_sets);
    print_result(number_of_sets, result_array, set_sizes);

    free(set_sizes);
    free_result_array(result_array, number_of_sets);
}

// Fills the array with parsed lines.
// Returns the pointer to it.
line_t *fill_line_array(unsigned int *size, size_t *number_of_line) {
    ssize_t line_size;
    char *input_line;
    size_t capacity = STARTING_SIZE;
    line_t *line_array = safe_malloc(capacity * sizeof(line_t));

    while ((input_line = get_line_from_input(&line_size)) != NULL) {
        line_t line = make_line(input_line, *number_of_line, line_size);
        (*number_of_line)++;

        if (*size + 1 == capacity) { // Resizing the array.
            capacity *= 2;
            line_array = safe_realloc(line_array, capacity * sizeof(line_t));
        }

        if (!line.is_comment && !line.is_error &&
            !line.is_empty) { // Adding only valid lines to array.
            sort_one_line(line);
            line_array[*size] = line;
            (*size)++;
        }
        free(input_line);
    }
    
    return line_array;
}

int main() {
    size_t number_of_line = 1;
    unsigned int size = 0;
    line_t *line_array = fill_line_array(&size, &number_of_line);

    if (size) {
        sort_all_lines(line_array, size);
        get_result(line_array, size, number_of_line);
    }

    free_line_array(line_array, size);

    return 0;
}