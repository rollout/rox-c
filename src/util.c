#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <stdio.h>
#include <time.h>

#include "util.h"
#include "base64.h"
#include "strrep.h"
#include "md5.h"

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
    size_t length = strlen(ptr);
    char *copy = malloc((length + 1) * sizeof(char));
    strncpy_s(copy, length + 1, ptr, length + 1);
    return copy;
}

int *ROX_INTERNAL mem_str_to_int(const char *str) {
    assert(str);
    long num = strtol(str, NULL, 0);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_int(num);
}

double *ROX_INTERNAL mem_str_to_double(const char *str) {
    assert(str);
    double num = strtod(str, NULL);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_double(num);
}

char *ROX_INTERNAL mem_int_to_str(int value) {
    char buffer[10];
    _itoa_s(value, buffer, 10, 10);
    return mem_copy_str(buffer);
}

char *ROX_INTERNAL mem_double_to_str(double value) {
    char buffer[20];
    sprintf_s(buffer, 20, "%f", value);
    return mem_copy_str(buffer);
}

char *ROX_INTERNAL mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value) {
    return value
           ? mem_copy_str(true_value)
           : mem_copy_str(false_value);
}

bool ROX_INTERNAL str_matches(const char *str, const char *pattern, int options) {

    int error_number;
    PCRE2_SIZE error_offset;
    pcre2_code *re = pcre2_compile(
            (PCRE2_SPTR) pattern,
            PCRE2_ZERO_TERMINATED,
            options,
            &error_number,
            &error_offset,
            NULL);

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(error_number, buffer, sizeof(buffer));
        // TODO: log
//        printf("PCRE2 compilation failed at offset %d: %s\n", (int)error_offset,
//               buffer);
        return false;
    }

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
    int rc = pcre2_match(
            re,
            (PCRE2_SPTR) str,
            strlen(str),
            0,                    /* start at offset 0 in the subject */
            0,                    /* default options */
            match_data,           /* block for storing the result */
            NULL);

    // TODO: handle errors?

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return rc >= 0;
}

int ROX_INTERNAL str_index_of(const char *str, char c) {
    assert(str);
    char *e = strchr(str, c);
    if (!e) {
        return -1;
    }
    return (int) (e - str);
}

bool ROX_INTERNAL str_equals(const char *str, const char *another) {
    assert(str);
    assert(another);
    return str == another || strcmp(str, another) == 0;
}

void ROX_INTERNAL str_substring_b(const char *str, int start, int len, char *buffer) {
    assert(str);
    assert(start >= 0);
    assert(len >= 0);
    assert(buffer);
    assert(start + len <= strlen(str));
    memcpy(buffer, str + start, len);
    buffer[len] = '\0';
}

char *ROX_INTERNAL mem_str_substring(const char *str, int start, int len) {
    assert(str);
    assert(start >= 0);
    assert(len >= 0);
    if (start + len > strlen(str)) {
        return NULL;
    }
    char *buffer = calloc(len + 1, sizeof(char));
    str_substring_b(str, start, len, buffer);
    return buffer;
}

char *ROX_INTERNAL mem_str_replace(const char *str, const char *search, const char *rep) {
    assert(str);
    assert(search);
    assert(rep);
    return strrep(str, search, rep);
}

char *ROX_INTERNAL mem_str_concat(const char *s1, const char *s2) {
    assert(s1);
    assert(s2);
    size_t len = strlen(s1) + strlen(s2);
    char *buffer = calloc(len + 1, sizeof(char));
    sprintf_s(buffer, len, "%s%s", s1, s2);
    return buffer;
}

long ROX_INTERNAL current_time_millis() {
    time_t t;
    time(&t);
    // TODO: get millis somehow
    return (long) (t * 1000);
}

char *ROX_INTERNAL mem_base64_encode(const char *s) {
    assert(s);
    char buffer[1024];
    size_t len = strlen(s);
    int result = base64encode(s, len * sizeof(char), buffer, 1024);
    assert(result == 0);
    return result == 0 ? mem_copy_str(buffer) : NULL;
}

char *ROX_INTERNAL mem_md5(const char *s) {
    assert(s);
    MD5_CTX context;
    unsigned char digest[16];
    size_t len = strlen(s);

    MD5_Init(&context);
    MD5_Update(&context, s, len);
    MD5_Final(digest, &context);

    char *result = malloc(33);
    static const char hexits[17] = "0123456789abcdef";
    int i;
    for (i = 0; i < 16; i++) {
        result[i * 2] = hexits[digest[i] >> 4];
        result[(i * 2) + 1] = hexits[digest[i] & 0x0F];
    }
    result[32] = '\0';
    return result;
}

char *ROX_INTERNAL mem_base64_decode(const char *s) {
    assert(s);
    unsigned char buffer[1024];
    size_t len = strlen(s);
    size_t resulting_str_len;
    int result = base64decode(s, len, buffer, &resulting_str_len);
    assert(result == 0);
    if (result == 0) {
        buffer[resulting_str_len] = 0;
        return mem_copy_str((char *) buffer);
    }
    return NULL;
}
