#ifndef SORTING_H
#define SORTING_H

#include "parsing.h"
#include <stddef.h>

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

int compare_non_negatives(const void *, const void *);
int compare_two_words(const void *, const void *);
void sort_one_line(line_t);
int compare_two_lines(const void *, const void *);
void sort_all_lines(line_t *, unsigned int);
void sort_set_array(size_t *, size_t);
int compare_non_negative_pointers(const void *, const void *);
void sort_result_array(size_t **, size_t);

#endif // SORTING_H