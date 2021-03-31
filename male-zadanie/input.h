#ifndef INPUT_H
#define INPUT_H

#include "parsing.h"
#include <stdbool.h>
#include <unistd.h>

bool is_valid_character(int);
bool is_line_valid(char *, ssize_t);
void change_to_lower_case(char *);
char *get_line_from_input(ssize_t *);
word_t *get_word_array(char *, size_t *);

#endif // INPUT_H