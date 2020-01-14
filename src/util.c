#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"

int *mem_copy_int(int value) {
    int *copy = malloc(sizeof(value));
    *copy = value;
    return copy;
}

double *mem_copy_double(double value) {
    double *copy = malloc(sizeof(value));
    *copy = value;
    return copy;
}

bool *mem_copy_bool(bool value) {
    bool *copy = malloc(sizeof(value));
    *copy = value;
    return copy;
}

char *ROX_INTERNAL mem_copy_str(const char *ptr) {
    assert(ptr);
    unsigned int length = strlen(ptr);
    char *copy = malloc((length + 1) * sizeof(char));
    strncpy_s(copy, length + 1, ptr, length + 1);
    return copy;
}

int *ROX_INTERNAL str_to_int(char *str) {
    assert(str);
    long num = strtol(str, NULL, 0);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_int(num);
}

double *ROX_INTERNAL str_to_double(char *str) {
    assert(str);
    double num = strtod(str, NULL);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_double(num);
}
