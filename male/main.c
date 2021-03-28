#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTING_SIZE 5
#define WHITE_DELIMITERS "\t\v\f\r \n"
#define FIRST_VALID_ASCII 33
#define LAST_VALID_ASCII 126

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
typedef struct Word_type {
    union word_union w_union;
    enum which_word_type which_type;
} word_t;

// A struct that holds all the information about one line - a pointer to an array containing
// all the words, number of the words, whether it's a comment or there's an error
// and a number of line.
typedef struct Line_type {
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

// Gets one line from input, returns the pointer to the array containing characters from the
// input and saves the size on a given variable.
char *get_line_from_input(size_t *size) {
    errno = 0;
    char *buffer;
    size_t buff_size = STARTING_SIZE;

    buffer = safe_malloc(buff_size * sizeof(char));

    *size = getline(&buffer, &buff_size, stdin);
    if (*size == -1) {
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

// TODO -0 jako negative xd
// TODO nan to string
// TODO ogarnac 0x
// TODO Przed liczbami ósemkowymi i szesnastkowymi nie może pojawić się znak +
// TODO wyifowac double osemkowe i szesnastkowe -> nie moga takie byc

void parse_one_word(char *start, word_t *word) {
    errno = 0;
    char *end = NULL;
    bool is_negative = false;

    if (*start == '-') {
        start++;
        is_negative = true;
    }

    unsigned long long value = strtoull(start, &end, 0);

    if ((*end != '\0') || (errno != 0) || (end == start)) {
        if (is_negative) {
            start--;
        }
        errno = 0;
        double value_d = strtod(start, &end);
        if ((errno != 0) || (end == start) || (*end != '\0')) {
            read_string(start, word);
        } else {
            word->w_union.floating_point_number = value_d;
            word->which_type = FLOATING_POINT_NUMBER;
        }
        return;
    }

    if (is_negative) {
        word->w_union.negative_number = (long long)(-1 * value);
        // TODO cast only gdy jest w zakresie else wczytać jako string
        word->which_type = NEGATIVE_NUMBER;
    } else {
        word->w_union.non_negative_number = value;
        word->which_type = NON_NEGATIVE_NUMBER;
    }
}

// Prints the given word. (its only needed for debugging purposes)
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

// Returns a pointer to an array containing all words from a given line.
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
        unsigned int j = 0;
        line.word_array = give_word_array(input_line, &j);
        line.number_of_words = j;
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

static inline int compare_negatives(long long a, long long b) {
    return (a > b) - (a < b);
}

static inline int compare_non_negatives(unsigned long long a, unsigned long long b) {
    return (a > b) - (a < b);
}

static inline int compare_floating_points(double a, double b) {
    return (a > b) - (a < b);
}

// TODO ladniejsze ify
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
        print_line(whole_line);

        if (i + 1 == allocated_size) {
            allocated_size *= 2;
            line_array = safe_realloc(line_array, allocated_size * sizeof(line_t));
        }
        line_array[i] = whole_line;

        i++;
        free(input_line);
        input_line = get_line_from_input(&line_size);
    }

    // sort_all_lines(line_array, i + 1);

    free_line_array(line_array, i);
    free(line_array);
    free(input_line);

    return 0;
}
