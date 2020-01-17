#pragma once

#include <stdbool.h>
#include "roxapi.h"

//
// Utility functions.
//
// NOTE: all functions prefixed with 'mem_' allocate memory
// and so returned pointer must be freed by the caller.
//
// On the contrary, all functions having '_b' suffix require buffer
// to be pre-allocated and passed as the (usually last) argument.
//

int *ROX_INTERNAL mem_copy_int(int value);

double *ROX_INTERNAL mem_copy_double(double value);

bool *ROX_INTERNAL mem_copy_bool(bool value);

char *ROX_INTERNAL mem_copy_str(const char *ptr);

int *ROX_INTERNAL mem_str_to_int(const char *str);

double *ROX_INTERNAL mem_str_to_double(const char *str);

char *ROX_INTERNAL mem_int_to_str(int value);

char *ROX_INTERNAL mem_double_to_str(double value);

char *ROX_INTERNAL mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value);

bool ROX_INTERNAL str_matches(const char *str, const char *pattern, int options);

int ROX_INTERNAL str_index_of(const char *str, char c);

bool ROX_INTERNAL str_equals(const char *str, const char *another);

void ROX_INTERNAL str_substring_b(const char *str, int start, int len, char *buffer);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 *
 * @param str The input string.
 * @param start The start offset.
 * @param len Length of the returned substring.
 * @return Pointer to the NEWLY CREATED string which is a substring of the given string or NULL in case where start offset or length is out of bounds.
 */
char *ROX_INTERNAL mem_str_substring(const char *str, int start, int len);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 *
 * @param str The input string.
 * @param search The string to replace.
 * @param rep The replacement.
 * @return Pointer to the NEWLY CREATED string in which all occurrences of <code>search</code> are replaced with <code>rep</code>.
 */
char *ROX_INTERNAL mem_str_replace(const char *str, const char *search, const char *rep);