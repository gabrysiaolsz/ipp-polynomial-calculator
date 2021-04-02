CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS =

.PHONY: all clean

all: similar_lines

similar_lines: main.o output.o sorting.o input.o parsing.o
parsing.o: parsing.c parsing.h safe_memory_allocation.h
input.o: input.c input.h safe_memory_allocation.h
sorting.o: sorting.c sorting.h parsing.h
output.o: output.c output.h input.h safe_memory_allocation.h sorting.h
main.o: main.c output.h

clean:
	rm -f *.o similar_lines