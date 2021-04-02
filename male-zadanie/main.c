#define _GNU_SOURCE
#include "output.h"

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