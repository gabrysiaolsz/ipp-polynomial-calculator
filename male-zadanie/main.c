#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTING_SIZE 5
#define WHITE_DELIMITERS "\t\v\f\r \n"
#define FIRST_VALID_ASCII 33
#define LAST_VALID_ASCII 126
#define ZERO '0'
#define HEXADECIMAL_PREFIX "0x"
#define VALID_HEXADECIMAL_CHARS "0123456789abcdf"
#define VALID_OCTAL_CHARS "01234567"
#define VALID_DECIMAL_CHARS "0123456789"
#define HEXADECIMAL_BASE 16
#define DECIMAL_BASE 10
#define OCTAL_BASE 8

// Union that covers all the possible data types needed to hold a line of input.
union word_union {
    long long negative_number;
    unsigned long long non_negative_number;
    double floating_point_number;
    char *not_a_number;
};

// Enumeration type saying which data type is stored in a union.
enum which_word_type {
    NEGATIVE_NUMBER,
    NON_NEGATIVE_NUMBER,
    FLOATING_POINT_NUMBER,
    NOT_A_NUMBER
};

// A struct that holds the data union and a enumeration type saying which type is in the union.
typedef struct word_type {
    union word_union w_union;
    enum which_word_type which_type;
} word_t;

// A struct that holds all the information about one line - a pointer to an array containing
// all the words, number of the words, whether it's a comment or there's an error
// and a number of line.
typedef struct line_type {
    word_t *word_array;
    bool is_comment;
    bool is_error;
    unsigned int number_of_words;
    unsigned int number_of_line;
} line_t;

// Checks whether malloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what malloc has returned.
static inline void *safe_malloc(unsigned int size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        exit(1);
    }
    return ptr;
}

// Checks whether realloc returned NULL - if yes, then the program will exit with exit code 1,
// otherwise returns what realloc has returned.
static inline void *safe_realloc(void *ptr, unsigned int size) {
    void *realloc_ptr = realloc(ptr, size);
    if (realloc_ptr == NULL) {
        exit(1);
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
    for (unsigned int i = 0; i < size; i++) {
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

// TODO ungetc i ogarniecie komentarzy?

// Gets one line from input, returns the pointer to the array containing characters from the
// input and saves the size on a given variable.
char *get_line_from_input(size_t *size) {
    errno = 0;
    char *buffer;
    size_t buff_size = STARTING_SIZE;

    buffer = safe_malloc(buff_size * sizeof(char));

    *size = getline(&buffer, &buff_size, stdin);
    if (*size == -1) {
        free(buffer);
        if (errno != 0) {
            printf("%s", strerror(errno));
            perror("Get line didn't work");
            exit(1);
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
    word->which_type = NOT_A_NUMBER;
}

bool is_character_in_base(int character, const int base) {
    switch (base) {
        case OCTAL_BASE:
            if (!strchr(VALID_OCTAL_CHARS, character)) {
                return false;
            } else {
                return true;
            }
        case DECIMAL_BASE:
            if (!strchr(VALID_DECIMAL_CHARS, character)) {
                return false;
            } else {
                return true;
            }
        case HEXADECIMAL_BASE:
            if (!strchr(VALID_HEXADECIMAL_CHARS, character)) {
                return false;
            } else {
                return true;
            }
    }
}

bool is_hexadecimal_number(const char *word) {
    unsigned int len = strlen(word);
    if (len < 2) {
        return false;
    }
    if (word[0] == '+' || word[0] == '-') {
        word++;
        len--;
    }
    if (strncmp(word, HEXADECIMAL_PREFIX, 2) != 0) { // First two characters aren't the hex_prefix
        return false;
    }
    for (unsigned int i = 2; i < len; i++) {
        if (!is_character_in_base((int)word[i], HEXADECIMAL_BASE)) {
            return false;
        }
    }
    return true;
}

bool is_valid_octal_number(const char *word) {
    if (*word != ZERO) {
        return false;
    }
    for (unsigned int i = 0; i < strlen(word); i++) {
        if (!is_character_in_base((int)word[i], OCTAL_BASE)) {
            return false;
        }
    }
    return true;
}

bool is_valid_decimal_number(const char *word) {
    unsigned int len = strlen(word);
    if (word[0] == '+' || word[0] == '-') {
        if (len > 1) {
            word++;
            len--;
        } else {
            return false;
        }
    }
    for (unsigned int i = 0; i < len; i++) {
        if (!is_character_in_base((int)word[i], DECIMAL_BASE)) {
            return false;
        }
    }
    return true;
}

void parse_one_word(char *input_word, word_t *parsed_word) {
    char *end = NULL;
    if (is_hexadecimal_number(input_word)) {
        if (input_word[0] != ZERO) {
            read_string(input_word, parsed_word);
        } else {
            errno = 0;
            unsigned long long hex_value = strtoull(input_word, &end, HEXADECIMAL_BASE);
            if (errno != 0) {
                read_string(input_word, parsed_word);
            } else {
                parsed_word->which_type = NON_NEGATIVE_NUMBER;
                parsed_word->w_union.non_negative_number = hex_value;
            }
        }
    } else if (is_valid_octal_number(input_word)) {
        errno = 0;
        unsigned long long oct_value = strtoull(input_word, &end, OCTAL_BASE);
        if (errno != 0) {
            read_string(input_word, parsed_word);
        } else {
            parsed_word->which_type = NON_NEGATIVE_NUMBER;
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
                    parsed_word->which_type = NON_NEGATIVE_NUMBER;
                    parsed_word->w_union.non_negative_number = (unsigned)neg_value;
                } else {
                    parsed_word->which_type = NEGATIVE_NUMBER;
                    parsed_word->w_union.negative_number = neg_value;
                }
            }
        } else {
            errno = 0;
            unsigned long long pos_value = strtoull(input_word, &end, DECIMAL_BASE);
            if (errno != 0) {
                read_string(input_word, parsed_word);
            } else {
                parsed_word->which_type = NON_NEGATIVE_NUMBER;
                parsed_word->w_union.non_negative_number = pos_value;
            }
        }
    } else {
        errno = 0;
        double duble = strtod(input_word, &end);
        if ((errno != 0) || (end == input_word) || (*end != '\0')) {
            read_string(input_word, parsed_word);
        } else if (isnan(duble)) {
            read_string("nan", parsed_word);
        } else {
            parsed_word->w_union.floating_point_number = duble;
            parsed_word->which_type = FLOATING_POINT_NUMBER;
        }
    }
}

// Prints the given word. (it's only needed for debugging purposes)
void print_word(word_t *word) {
    if (word->which_type == NEGATIVE_NUMBER)
        printf("negative: %lld\n", word->w_union.negative_number);
    else if (word->which_type == NON_NEGATIVE_NUMBER)
        printf("non negative: %llu\n", word->w_union.non_negative_number);
    else if (word->which_type == FLOATING_POINT_NUMBER)
        printf("floating: %f\n", word->w_union.floating_point_number);
    else
        printf("string: %s\n", word->w_union.not_a_number);
}

// Returns a pointer to an array containing all words from a given input line.
// Sets the given variable to number of words in the line.
word_t *give_word_array(char *input_line, unsigned int *number_of_elements) {
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
    line.number_of_words = 0;
    return line;
}

// Checks whether a line is a comment or valid and sets corresponding boolean variables
// accordingly.
// If it is a valid, non-comment line, fills the array with words from a given line of input.
line_t make_line(char *input_line, unsigned int number_of_line, size_t size) {
    line_t line = initialise_line();
    line.number_of_line = number_of_line;

    if (input_line[0] == '#') {
        line.is_comment = true;
    } else if (!is_line_valid(input_line, size)) {
        line.is_error = true;
    } else {
        unsigned int number_of_elements = 0;
        line.word_array = give_word_array(input_line, &number_of_elements);
        line.number_of_words = number_of_elements;
    }
    return line;
}

// Prints the given line. (it's only for debugging purposes)
void print_line(line_t line) {
    printf("line %d ", line.number_of_line);
    if (line.is_error) {
        printf("line is an error\n");
    } else if (line.is_comment) {
        printf("line is a comment\n");
    } else {
        for (unsigned int i = 0; i < line.number_of_words; i++) {
            print_word(&line.word_array[i]);
        }
    }
}

// Compares given negative numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_negatives(long long a, long long b) {
    return (a > b) - (a < b);
}

// Compares given non-negative numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_non_negatives(unsigned long long a, unsigned long long b) {
    return (a > b) - (a < b);
}

// Compares given floating-point numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
static inline int compare_floating_points(double a, double b) {
    return (a > b) - (a < b);
}

// Compares
int compare_two_words(const void *p, const void *q) {
    word_t word1 = *(const word_t *)p;
    word_t word2 = *(const word_t *)q;

    if (word1.which_type != word2.which_type) {
        return (word1.which_type > word2.which_type) - (word1.which_type < word2.which_type);
    } else { // They're the same type.
        switch (word1.which_type) {
            case NEGATIVE_NUMBER:
                return compare_negatives(word1.w_union.negative_number,
                                         word2.w_union.negative_number);
            case NON_NEGATIVE_NUMBER:
                return compare_non_negatives(word1.w_union.non_negative_number,
                                             word2.w_union.non_negative_number);
            case FLOATING_POINT_NUMBER:
                return compare_floating_points(word1.w_union.floating_point_number,
                                               word2.w_union.floating_point_number);
            case NOT_A_NUMBER:
                return strcmp(word1.w_union.not_a_number, word2.w_union.not_a_number);
        }
    }
}

// Sorts the elements of one line.
void sort_one_line(line_t line_to_sort) {
    qsort(line_to_sort.word_array, line_to_sort.number_of_words, sizeof(word_t),
          compare_two_words);
}

//
int compare_two_lines(const void *p, const void *q) {
    line_t line1 = *(const line_t *)p;
    line_t line2 = *(const line_t *)q;

    if (line1.number_of_words < line2.number_of_words) {
        return -1;
    } else if (line1.number_of_words == line2.number_of_words) {
        for (unsigned int i = 0; i < line1.number_of_words; i++) {
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
        for (unsigned int j = 0; j < line_array[i].number_of_words; j++) {
            if (line_array[i].word_array[j].which_type == NOT_A_NUMBER) {
                free(line_array[i].word_array[j].w_union.not_a_number);
            }
        }
        free(line_array[i].word_array);
    }
    free(line_array);
}

int main() {
    size_t line_size;
    char *input_line = get_line_from_input(&line_size);
    unsigned int i = 0, allocated_size = STARTING_SIZE;
    line_t *line_array = safe_malloc(STARTING_SIZE * sizeof(line_t));

    while (input_line != NULL) {
        change_to_lower_case(input_line);

        line_t whole_line = make_line(input_line, i + 1, line_size);
        sort_one_line(whole_line);
        // print_line(whole_line);

        if (i + 1 == allocated_size) {
            allocated_size *= 2;
            line_array = safe_realloc(line_array, allocated_size * sizeof(line_t));
        }
        line_array[i] = whole_line;

        i++;
        free(input_line);
        input_line = get_line_from_input(&line_size);
    }

    // TODO spytac sie o to jakuba
    sort_all_lines(line_array, i);

    unsigned int **big_result_array;
    big_result_array = malloc(STARTING_SIZE * sizeof(unsigned int));
    unsigned int big_index = 0, small_index = 0, big_allocated = STARTING_SIZE,
                 small_allocated = 1;
    for (int pom = 0; pom < big_allocated; pom++) {
        big_result_array[pom] = safe_malloc(small_allocated * sizeof(unsigned int));
    }

    big_result_array[big_index][small_index] = line_array[0].number_of_line;

    for (unsigned int k = 0; k < i - 1; k++) {
        const void *line1_ptr = &line_array[k];
        const void *line2_ptr = &line_array[k + 1];
        int res = compare_two_lines(line1_ptr, line2_ptr);

        if (big_index == big_allocated) {
            big_allocated *= 2;
            big_result_array =
                safe_realloc(big_result_array, big_allocated * sizeof(unsigned int));
        }
        if (small_index == small_allocated) {
            small_allocated *= 2;
            big_result_array[big_index] =
                safe_realloc(big_result_array[big_index], small_allocated * sizeof(unsigned int));
        }
        if (res == 0) {
            big_result_array[big_index][small_index] = line_array[k + 1].number_of_line;
            small_index++;
        } else { // niepodobne
            big_index++;
            small_index = 0;
            big_result_array[big_index][small_index] = line_array[k].number_of_line;
        }
    }

    for (int pom2 = 0; pom2 < big_allocated; pom2++) {
        free(big_result_array[pom2]);
    }

    free(big_result_array);

    free_line_array(line_array, i);
    free(input_line);

    return 0;
}
