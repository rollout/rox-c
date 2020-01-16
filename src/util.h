#pragma once

#include <stdbool.h>
#include "roxapi.h"

int *ROX_INTERNAL mem_copy_int(int value);

double *ROX_INTERNAL mem_copy_double(double value);

bool *ROX_INTERNAL mem_copy_bool(bool value);

char *ROX_INTERNAL mem_copy_str(const char *ptr);

int *ROX_INTERNAL str_to_int(char *str);

double *ROX_INTERNAL str_to_double(char *str);

bool ROX_INTERNAL str_matches(const char *str, const char *pattern, int options);

int ROX_INTERNAL str_index_of(const char *str, const char c);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 *
 * @param str The input string.
 * @param start The start offset.
 * @param len Length of the str_substring.
 * @return Pointer to the NEWLY CREATED string which is a str_substring of the given string or NULL in case where start offset or length is out of bounds.
 */
char *ROX_INTERNAL str_substring(const char *str, int start, int len);