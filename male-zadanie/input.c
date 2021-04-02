#define _GNU_SOURCE
#include "input.h"
#include "safe_memory_allocation.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define WHITE_DELIMITERS "\t\v\f\r \n"
#define FIRST_VALID_ASCII 33
#define LAST_VALID_ASCII 126
#define STARTING_SIZE 5

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