#pragma once

#include <ctype.h>
#include <stdio.h>

#define DEFAULT_API_KEY "5e6a3533d3319d76d1ca33fd"
#define DEFAULT_DEV_MODE_KEY "297c23e7fcb68e54c513dcca"

static inline char prompt(const char *message) {
    printf("%s\n", message);
    char c = fgetc(stdin);
    if ((c == '\n' || c == EOF)) { // Enter key pressed
        c = 'y';
    } else {
        getchar(); // read dummy character to clear input buffer, which inserts after character input
    }
    printf("\n");
    return tolower(c);
}