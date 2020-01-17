#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <stdio.h>

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

int ROX_INTERNAL str_index_of(const char *str, const char c) {
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

/*
 * This function returns string <code>str</code> if string <code>search</code> is an empty string, or
 * if <code>search</code> is not found in <code>str</code>. If <code>search</code> is found in <code>str</code>, the function
 * returns a new null-terminated string whose contents are identical
 * to <code>str</code>, except that all occurrences of <code>search</code> in the original string <code>str</code>
 * are, in the new string, replaced by the string <code>rep</code>. The caller owns
 * the new string.
 *
 * Strings <code>str</code>, <code>search</code>, and <code>rep</code> must all be null-terminated strings. If any
 * of <code>str</code>, <code>search</code>, or <code>rep</code> are NULL, the function returns NULL, indicating an
 * error condition. If any other error occurs, the function returns
 * NULL.
 *
 * This code is written pedantically, primarily so that asserts can be
 * used liberally. The code could certainly be optimized and/or made
 * less verbose, and I encourage you to do that if you use strstr in
 * your production code, once you're comfortable that it functions as
 * intended. Each assert makes plain an invariant condition that is
 * assumed to be true by the statement(s) that immediately follow the
 * assert.  Some of the asserts are trivially true, as written, but
 * they're included, nonetheless, in case you, in the process of
 * optimizing or adapting the code for your own purposes, make a
 * change that breaks an assumption made downstream by the original
 * code.
 *
 * NOTE: the function code and the above comment is taken from here:
 * https://gist.github.com/dhess/975639/bb91cd552c0a92306b8ef49b417c6796f67036ce
 */
char *ROX_INTERNAL mem_str_replace(const char *str, const char *search, const char *rep) {
    if (!str || !search || !rep)
        return 0;
    size_t s1_len = strlen(str);
    if (!s1_len)
        return (char *) str;
    size_t s2_len = strlen(search);
    if (!s2_len)
        return (char *) str;

    /*
     * Two-pass approach: figure out how much space to allocate for
     * the new string, pre-allocate it, then perform replacement(s).
     */

    size_t count = 0;
    const char *p = str;
    assert(s2_len); /* otherwise, strstr(<code>str</code>,<code>search</code>) will return <code>str</code>. */
    do {
        p = strstr(p, search);
        if (p) {
            p += s2_len;
            ++count;
        }
    } while (p);

    if (!count)
        return (char *) str;

    /*
     * The following size arithmetic is extremely cautious, to guard
     * against size_t overflows.
     */
    assert(s1_len >= count * s2_len);
    assert(count);
    size_t s1_without_s2_len = s1_len - count * s2_len;
    size_t s3_len = strlen(rep);
    size_t s1_with_s3_len = s1_without_s2_len + count * s3_len;
    if (s3_len &&
        ((s1_with_s3_len <= s1_without_s2_len) || (s1_with_s3_len + 1 == 0)))
        /* Overflow. */
        return 0;

    char *s1_with_s3 = (char *) malloc(s1_with_s3_len + 1); /* w/ terminator */
    if (!s1_with_s3)
        /* ENOMEM, but no good way to signal it. */
        return 0;

    char *dst = s1_with_s3;
    const char *start_substr = str;
    size_t i;
    for (i = 0; i != count; ++i) {
        const char *end_substr = strstr(start_substr, search);
        assert(end_substr);
        size_t substr_len = end_substr - start_substr;
        memcpy(dst, start_substr, substr_len);
        dst += substr_len;
        memcpy(dst, rep, s3_len);
        dst += s3_len;
        start_substr = end_substr + s2_len;
    }

    /* copy remainder of <code>str</code>, including trailing '\0' */
    size_t remains = s1_len - (start_substr - str) + 1;
    assert(dst + remains == s1_with_s3 + s1_with_s3_len + 1);
    memcpy(dst, start_substr, remains);
    assert(strlen(s1_with_s3) == s1_with_s3_len);
    return s1_with_s3;
}