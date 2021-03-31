#include "output.h"
#include "input.h"
#include "safe_memory_allocation.h"
#include <stdio.h>

#define STARTING_SIZE 5

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

// Prints the result.
void print_result(size_t number_of_sets, size_t **result_array, const size_t *set_sizes) {
    for (unsigned int k = 0; k < number_of_sets; k++) {
        size_t set_cardinality = set_sizes[result_array[k][0] - 1];
        print_array(result_array[k], set_cardinality);
    }
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
