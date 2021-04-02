#include "sorting.h"
#include <stdlib.h>
#include <string.h>

// Compares given non-negative integers numbers and returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
int compare_non_negatives(const void *a, const void *b) {
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

// Sorts the set array.
void sort_set_array(size_t *set_array, size_t size) {
    qsort(set_array, size, sizeof(size_t), compare_non_negatives);
}

// Compares pointers to non-negative values. Returns -1/0/1 when the first one is
// smaller/equal/bigger than the second one.
int compare_non_negative_pointers(const void *a, const void *b) {
    const unsigned int *first = *(const unsigned int **)a;
    const unsigned int *second = *(const unsigned int **)b;

    return (*first > *second) - (*first < *second);
}

// Sorts a 2D array containing result.
void sort_result_array(size_t **result_array, size_t number_of_sets) {
    qsort(result_array, number_of_sets, sizeof(size_t *), compare_non_negative_pointers);
}
