#ifndef MIKA_STD_H
#define MIKA_STD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define print printf

int input(void);

void input_string(char* buffer, int size);

int power(int base, int exponent);

int absolute(int number);

#define string_length strlen

#define string_compare strcmp

#define string_copy strcpy

int* array_create(int size);

void array_free(int* array);

int array_size(int* array, int size);

#endif
