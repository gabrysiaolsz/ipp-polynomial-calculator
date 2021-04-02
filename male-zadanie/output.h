#ifndef OUTPUT_H
#define OUTPUT_H

#include "sorting.h"
#include <unistd.h>

line_t make_line(char *, size_t, ssize_t);
void free_line_array(line_t *, unsigned int);
size_t get_number_of_sets(line_t *, unsigned int);
size_t get_set_cardinality(line_t *, size_t, unsigned int);
void free_result_array(size_t **, size_t);
void print_array(size_t *, size_t);
void print_result(size_t, size_t **, const size_t *);
void get_result(line_t *, unsigned int, size_t);
line_t *fill_line_array(unsigned int *, size_t *);

#endif // OUTPUT_H