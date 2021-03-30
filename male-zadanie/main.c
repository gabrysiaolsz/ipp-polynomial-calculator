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
#define INVALID_HEXADECIMAL_CHARS "p."
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
    char *not_a_number;
};

// Enumeration type saying which data type is stored in a union.
enum enum_word_type { NEGATIVE_NUMBER, NON_NEGATIVE_NUMBER, FLOATING_POINT_NUMBER, NOT_A_NUMBER };

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
    unsigned int words_count;
    unsigned int line_number;
    bool is_comment;
    bool is_error;
    bool is_empty;
} line_t;

// Checks whether malloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what malloc has returned.
static inline void *safe_malloc(unsigned int size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Checks whether realloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what realloc has returned.
static inline void *safe_realloc(void *ptr, unsigned int size) {
    void *realloc_ptr = realloc(ptr, size);
    if (realloc_ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return realloc_ptr;
}

// Returns true when the given character is valid (is within ASCII 33-126 or whitespace)
// false otherwise.
bool is_valid_character(int character) {
    return ((character >= FIRST_VALID_ASCII && character <= LAST_VALID_ASCII) ||
            isspace(character));
}

// Returns true if given line is valid, false otherwise.
bool is_line_valid(char *line, size_t size) {
    for (size_t i = 0; i < size; i++) {
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
char *get_line_from_input(size_t *size) {
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

// Reads string and saves it to a given word.
void read_string(char *string, word_t *word) {
    word->w_union.not_a_number = safe_malloc((strlen(string) + 1) * sizeof(word_t));
    strcpy(word->w_union.not_a_number, string);
    word->type = NOT_A_NUMBER;
}

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
    }
}

bool are_characters_valid_in_given_base(const char *word, int base) {
    while (*word) {
        if (!is_character_in_base(*word, base)) {
            return false;
        }
        word++;
    }
    return true;
}

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

bool is_valid_octal_number(const char *word) {
    if (*word != ZERO_ASCII) {
        return false;
    }
    return are_characters_valid_in_given_base(word, OCTAL_BASE);
}

bool is_valid_decimal_number(const char *word) {
    if (word[0] == '+' || word[0] == '-') {
        if (strlen(word) > 1) {
            word++;
        } else {
            return false;
        }
    }
    return are_characters_valid_in_given_base(word, DECIMAL_BASE);
}

bool is_only_whitespaces(const char *word) {
    return are_characters_valid_in_given_base(word, WHITE_BASE);
}

void parse_one_word(char *input_word, word_t *parsed_word) {
    char *end = NULL;
    if (is_hexadecimal_number(input_word)) {
        if (input_word[0] != ZERO_ASCII) {
            read_string(input_word, parsed_word);
        } else {
            errno = 0;
            unsigned long long hex_value = strtoull(input_word, &end, HEXADECIMAL_BASE);
            if (errno != 0) {
                read_string(input_word, parsed_word);
            } else {
                parsed_word->type = NON_NEGATIVE_NUMBER;
                parsed_word->w_union.non_negative_number = hex_value;
            }
        }
    } else if (is_valid_octal_number(input_word)) {
        errno = 0;
        unsigned long long oct_value = strtoull(input_word, &end, OCTAL_BASE);
        if (errno != 0) {
            read_string(input_word, parsed_word);
        } else {
            parsed_word->type = NON_NEGATIVE_NUMBER;
            parsed_word->w_union.non_negative_number = oct_value;
        }
    } else if (is_valid_decimal_number(input_word)) {
        if (input_word[0] == '-') {
            errno = 0;
            long long neg_value = strtoll(input_word, &end, DECIMAL_BASE);
            if (errno != 0) {
                read_string(input_word, parsed_word);
            } else {
                if (neg_value == 0) {
                    parsed_word->type = NON_NEGATIVE_NUMBER;
                    parsed_word->w_union.non_negative_number = (unsigned)neg_value;
                } else {
                    parsed_word->type = NEGATIVE_NUMBER;
                    parsed_word->w_union.negative_number = neg_value;
                }
            }
        } else {
            errno = 0;
            unsigned long long pos_value = strtoull(input_word, &end, DECIMAL_BASE);
            if (errno != 0) {
                read_string(input_word, parsed_word);
            } else {
                parsed_word->type = NON_NEGATIVE_NUMBER;
                parsed_word->w_union.non_negative_number = pos_value;
            }
        }
    } else {
        if (strlen(input_word) > 2 && input_word[0] == ZERO_ASCII && input_word[1] == X_ASCII) {
            are_characters_valid_in_given_base(input_word, INVALID_HEXADECIMAL_BASE);
            read_string(input_word, parsed_word);
            return;
        }
        if (strlen(input_word) > 3 && (input_word[0] == '+' || input_word[0] == '-') &&
            input_word[1] == ZERO_ASCII && input_word[2] == X_ASCII) {
            are_characters_valid_in_given_base(input_word, INVALID_HEXADECIMAL_BASE);
            read_string(input_word, parsed_word);
            return;
        }
        errno = 0;
        long double value_d = strtold(input_word, &end); // 7e1100
        if ((errno != 0) || (end == input_word) || (*end != '\0')) {
            read_string(input_word, parsed_word);
        } else if (value_d != value_d) {
            read_string("nan", parsed_word);
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
}

// Prints the given word. it's only needed for debugging purposes)
void print_word(word_t *word) {
    if (word->type == NEGATIVE_NUMBER)
        printf("negative: %lld\n", word->w_union.negative_number);
    else if (word->type == NON_NEGATIVE_NUMBER)
        printf("non negative: %llu\n", word->w_union.non_negative_number);
    else if (word->type == FLOATING_POINT_NUMBER)
        printf("floating: %Lf\n", word->w_union.floating_point_number);
    else
        printf("string: %s\n", word->w_union.not_a_number);
}

// Returns a pointer to an array containing all words from a given input line.
// Sets the given variable to number of words in the line.
word_t *get_word_array(char *input_line, unsigned int *number_of_elements) {
    word_t *array = safe_malloc(STARTING_SIZE * sizeof(word_t));
    unsigned int i = 0, allocated_size = STARTING_SIZE;
    word_t parsed_word;
    char *word_from_line = strtok(input_line, WHITE_DELIMITERS);

    while (word_from_line != NULL) {
        if (i == allocated_size) {
            allocated_size *= 2;
            array = safe_realloc(array, (allocated_size * sizeof(word_t)));
        }

        parse_one_word(word_from_line, &parsed_word);
        array[i] = parsed_word;
        i++;
        word_from_line = strtok(NULL, WHITE_DELIMITERS);
    }
    *number_of_elements = i;

    return array;
}

// Returns line with initialised values.
line_t initialise_line() {
    line_t line;
    line.word_array = NULL;
    line.is_comment = false;
    line.is_error = false;
    line.is_empty = false;
    line.words_count = 0;
    return line;
}

// Checks whether a line is a comment or valid and sets corresponding boolean variables
// accordingly.
// If it is a valid, non-comment line, fills the array with words from a given line of input.
line_t make_line(char *input_line, unsigned int number_of_line, size_t size) {
    line_t line = initialise_line();
    line.line_number = number_of_line;

    if (input_line[0] == '#') {
        line.is_comment = true;
    }
    if (!is_line_valid(input_line, size) && !line.is_comment) {
        line.is_error = true;
        fprintf(stderr, "%s %u\n", "ERROR", line.line_number);
    }
    if (is_only_whitespaces(input_line)) {
        line.is_empty = true;
    }
    if (!line.is_comment && !line.is_error && !line.is_empty) {
        unsigned int number_of_elements = 0;
        line.word_array = get_word_array(input_line, &number_of_elements);
        line.words_count = number_of_elements;
    }
    return line;
}

// Prints the given line. (it's only for debugging purposes)
void print_line(line_t line) {
    printf("line %d ", line.line_number);
    if (line.is_error) {
        printf("line is an error\n");
    } else if (line.is_comment) {
        printf("line is a comment\n");
    } else {
        for (unsigned int i = 0; i < line.words_count; i++) {
            print_word(&line.word_array[i]);
        }
    }
}

// Compares given non-negative integers numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_non_negatives(const void *a, const void *b) {
    unsigned int first = *(const unsigned int *)a;
    unsigned int second = *(const unsigned int *)b;
    return (first > second) - (first < second);
}

// Compares
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
        case NOT_A_NUMBER:
            return strcmp(word1.w_union.not_a_number, word2.w_union.not_a_number);
    }
}

// Sorts the elements of one line.
void sort_one_line(line_t line) {
    qsort(line.word_array, line.words_count, sizeof(word_t), compare_two_words);
}

//
int compare_two_lines(const void *p, const void *q) {
    line_t line1 = *(const line_t *)p;
    line_t line2 = *(const line_t *)q;

    if (line1.words_count < line2.words_count) {
        return -1;
    } else if (line1.words_count == line2.words_count) {
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
        return 1;
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
            if (line_array[i].word_array[j].type == NOT_A_NUMBER) {
                free(line_array[i].word_array[j].w_union.not_a_number);
            }
        }
        free(line_array[i].word_array);
    }
    free(line_array);
}

unsigned int get_number_of_sets(line_t *line_array, unsigned int size) {
    unsigned int number = 0;
    for (unsigned int i = 0; i < size - 1; i++) {
        const void *line1_ptr = &line_array[i];
        const void *line2_ptr = &line_array[i + 1];

        int res = compare_two_lines(line1_ptr, line2_ptr);
        if (res != 0) {
            number++;
        }
    }
    return number + 1;
}

unsigned int get_set_cardinality(line_t *line_array, unsigned int set_start_index,
                                 unsigned int size) {
    unsigned int size_of_set = 1;
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

void sort_set_array(unsigned int *set_array, size_t size) {
    qsort(set_array, size, sizeof(unsigned int), compare_non_negatives);
}

void free_result_array(unsigned int **result_array, unsigned int number_of_sets) {
    for (unsigned int i = 0; i < number_of_sets; i++) {
        free(result_array[i]);
    }
    free(result_array);
}

void print_array(unsigned int *array, unsigned int size) {
    printf("%d", array[0]);
    for (unsigned k = 1; k < size; k++) {
        printf(" %d", array[k]);
    }
    printf("\n");
}

static inline int compare_non_negative_pointers(const void *a, const void *b) {
    const unsigned int *first = *(const unsigned int **)a;
    const unsigned int *second = *(const unsigned int **)b;

    return (*first > *second) - (*first < *second);
}

void make_print_result_array(line_t *line_array, unsigned int line_array_size,
                             unsigned int max_line_num) {
    size_t number_of_sets = get_number_of_sets(line_array, line_array_size);
    unsigned int set_index = 0, line_index = 0;
    unsigned int **result_array = safe_malloc(number_of_sets * sizeof(unsigned int *));
    unsigned int *set_sizes = calloc(max_line_num, sizeof(unsigned int));

    for (unsigned int i = 0; i < number_of_sets; i++) {
        unsigned int set_cardinality =
            get_set_cardinality(line_array, set_index, line_array_size);
        result_array[i] = safe_malloc(set_cardinality * sizeof(unsigned int));
        for (unsigned int j = 0; j < set_cardinality; j++) {
            result_array[i][j] = line_array[line_index].line_number;
            line_index++;
        }
        sort_set_array(result_array[i], set_cardinality);
        set_sizes[result_array[i][0] - 1] = set_cardinality;
        set_index += set_cardinality;
    }

    qsort(result_array, number_of_sets, sizeof(unsigned int *), compare_non_negative_pointers);

    set_index = 0;

    for (unsigned int k = 0; k < number_of_sets; k++) {
        unsigned int a = result_array[k][0];
        unsigned int set_cardinality = set_sizes[a - 1];
        print_array(result_array[k], set_cardinality);
        set_index += set_cardinality;
    }

    free(set_sizes);
    free_result_array(result_array, number_of_sets);
}

int main() {
    size_t line_size;
    char *input_line = get_line_from_input(&line_size);
    unsigned int number_of_line = 1, allocated_size = STARTING_SIZE, array_index = 0;
    line_t *line_array = safe_malloc(STARTING_SIZE * sizeof(line_t));

    while (input_line != NULL) {
        change_to_lower_case(input_line);

        line_t whole_line = make_line(input_line, number_of_line, line_size);
        number_of_line++;
        sort_one_line(whole_line);

        if (array_index + 1 == allocated_size) {
            allocated_size *= 2;
            line_array = safe_realloc(line_array, allocated_size * sizeof(line_t));
        }

        if (!whole_line.is_comment && !whole_line.is_error && !whole_line.is_empty) {
            line_array[array_index] = whole_line;
            array_index++;
        }

        // print_line(whole_line);

        free(input_line);
        input_line = get_line_from_input(&line_size);
    }

    if (array_index > 0) {
        sort_all_lines(line_array, array_index);

        make_print_result_array(line_array, array_index, number_of_line);
    }

    free_line_array(line_array, array_index);
    free(input_line);

    return 0;
}