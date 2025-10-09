#include "mika_std.h"

int input(void) {
    int value;
    printf(" ");
    scanf("%d", &value);
    return value;
}

void input_string(char* buffer, int size) {
    printf("");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    fgets(buffer, size, stdin);
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
}

int power(int base, int exponent) {
    int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

int absolute(int number) {
    return (number < 0) ? -number : number;
}

int* array_create(int size) {
    return (int*)malloc(size * sizeof(int));
}

void array_free(int* array) {
    free(array);
}

int array_size(int* array, int size) {
    return size;
}
