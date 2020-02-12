#pragma once

#include <cjson/cJSON.h>
#include <collectc/list.h>
#include <collectc/hashset.h>
#include <collectc/hashtable.h>
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

void *ROX_INTERNAL mem_copy(void *ptr, size_t bytes);

int *ROX_INTERNAL mem_copy_int(int value);

double *ROX_INTERNAL mem_copy_double(double value);

char *ROX_INTERNAL mem_copy_str(const char *ptr);

bool *ROX_INTERNAL mem_copy_bool(bool value);

HashTable *ROX_INTERNAL mem_copy_map(HashTable *map);

HashTable *ROX_INTERNAL mem_deep_copy_str_value_map(HashTable *map);

int *ROX_INTERNAL mem_str_to_int(const char *str);

double *ROX_INTERNAL mem_str_to_double(const char *str);

char *ROX_INTERNAL mem_int_to_str(int value);

char *ROX_INTERNAL mem_double_to_str(double value);

char *ROX_INTERNAL mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value);

bool ROX_INTERNAL str_matches(const char *str, const char *pattern, unsigned int options);

int ROX_INTERNAL str_index_of(const char *str, char c);

bool ROX_INTERNAL str_starts_with(const char *str, const char *prefix);

bool ROX_INTERNAL str_equals(const char *str, const char *another);

bool ROX_INTERNAL str_eq_n(const char *str, int start, int end, const char *another);

bool ROX_INTERNAL str_is_empty(const char *str);

/**
 * Note the passed <code>str</code> is modified in-place, without creating new strings.
 *
 * @param str Not <code>NULL</code>.
 * @return <code>str</code> itself, no new string is created.
 */
char *ROX_INTERNAL str_to_upper(char *str);

bool ROX_INTERNAL str_in_list(const char *str, List *list_of_strings);

void ROX_INTERNAL str_substring_b(const char *str, int start, int len, char *buffer);

size_t ROX_INTERNAL str_copy_value_to_buffer(char *buffer, size_t buffer_size, const char *value);

char *ROX_INTERNAL str_format_b(char *buffer, size_t buffer_size, const char *fmt, ...);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string. <em>Must</em> be zero-terminated.
 * @param start The start offset.
 * @param len Length of the returned substring.
 * @return Pointer to the NEWLY CREATED string which is a substring of the given string or NULL in case where start offset or length is out of bounds.
 */
char *ROX_INTERNAL mem_str_substring(const char *str, int start, int len);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string. Could be non-zero-terminated.
 * @param str_len The length of the given <code>str</code>.
 * @param start The start offset.
 * @param len Length of the returned substring.
 * @return Pointer to the NEWLY CREATED string which is a substring of the given string or NULL in case where start offset or length is out of bounds.
 */
char *ROX_INTERNAL mem_str_substring_n(const char *str, size_t str_len, int start, int len);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string.
 * @param search The string to replace.
 * @param rep The replacement.
 * @return Pointer to the NEWLY CREATED string in which all occurrences of <code>search</code> are replaced with <code>rep</code>.
 */
char *ROX_INTERNAL mem_str_replace(const char *str, const char *search, const char *rep);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s1 The left part. MUST NOT BE NULL.
 * @param s2 The right part. MUST NOT BE NULL.
 * @return Pointer to the NEWLY CREATED string which is a concatenation of the given two strings.
 */
char *ROX_INTERNAL mem_str_concat(const char *s1, const char *s2);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param fmt Format string. MUST NOT BE NULL.
 * @return Pointer to the NEWLY CREATED string which is a formatted string.
 */
char *ROX_INTERNAL mem_str_format(const char *fmt, ...);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @return Pointer to the NEWLY CREATED string which is a formatted string.
 */
char *ROX_INTERNAL mem_build_url(const char *base_uri, const char *path);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The string to encode.
 * @return Pointer to the NEWLY CREATED string which is a base64-encoded version of the given string.
 */
char *ROX_INTERNAL mem_base64_encode(const char *s);

/**
 * @param s The BASE64-ed string to decode.
 * @return Size if the resulting decoded data in bytes.
 */
size_t ROX_INTERNAL base64_decode_b(const char *s, unsigned char *buffer);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The BASE64-ed string to decode.
 * @return Pointer to the NEWLY CREATED string which is a base64-decoded version of the given string.
 */
char *ROX_INTERNAL mem_base64_decode_str(const char *s);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The input string. Not <code>NULL</code>.
 * @param buffer Buffer to output result to. Must not be <code>NULL</code>. Must be of length 16 (at least).
 */
void ROX_INTERNAL md5_str_b(const char *s, unsigned char *buffer);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The input string. Not <code>NULL</code>.
 * @return Pointer to the NEWLY CREATED string which is a binary md5 hash of the given string.
 */
char *ROX_INTERNAL mem_md5_str(const char *s);

unsigned char *ROX_INTERNAL mem_sha256(const char *s);

char *ROX_INTERNAL mem_sha256_str(const char *s);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param separator Separator string. Not <code>NULL</code>.
 * @param strings List of <code>char *</code>. Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL mem_str_join(const char *separator, List *strings);

/**
 * @return Number of milliseconds since Unix Epoch.
 */
double ROX_INTERNAL current_time_millis();

/**
 * @param file_path Path to the file to read. Not <code>NULL</code>.
 * @param buffer Not <code>NULL</code>.
 * @param buffer_size Not negative.
 * @return Number of bytes read or -1 in case of an error.
 */
size_t ROX_INTERNAL rox_file_read_b(const char *file_path, unsigned char *buffer, size_t buffer_size);

void ROX_INTERNAL rox_json_serialize(char *buffer, size_t buffer_size, unsigned int options, ...);

List *ROX_INTERNAL rox_list_create(void *skip, ...);

List *ROX_INTERNAL rox_list_create_str(void *skip, ...);

HashSet *ROX_INTERNAL rox_set_create(void *skip, ...);

HashTable *ROX_INTERNAL rox_map_create(void *skip, ...);

void ROX_INTERNAL rox_map_free_with_values(HashTable *map);

void ROX_INTERNAL rox_map_free_with_values_cb(HashTable *map, void (*f)(void *));

void ROX_INTERNAL rox_map_free_with_keys_and_values(HashTable *map);

void ROX_INTERNAL
rox_hash_table_free_with_keys_and_values_cb(HashTable *map, void (*f_key)(void *), void (*f_value)(void *));

#define ROX_JSON_PRETTY_PRINT 1u

#define ROX_JSON_SERIALIZE(buffer, buffer_size, ...) rox_json_serialize(buffer, buffer_size, 0, __VA_ARGS__, NULL)

#define ROX_JSON_SERIALIZE_PRETTY(buffer, buffer_size, ...) rox_json_serialize(buffer, buffer_size, ROX_JSON_PRETTY_PRINT, __VA_ARGS__, NULL)

#define ROX_JSON_STRING(value) cJSON_CreateString(value)

#define ROX_JSON_INT(value) cJSON_CreateNumber(value)

#define ROX_JSON_DOUBLE(value) cJSON_CreateNumber(value)

#define ROX_JSON_TRUE cJSON_CreateTrue()

#define ROX_JSON_FALSE cJSON_CreateFalse()

#define ROX_JSON_BOOL cJSON_CreateBool()

#define ROX_JSON_NULL cJSON_CreateNull(value)

#define ROX_LIST(...) rox_list_create(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_LIST ROX_LIST(NULL)

#define ROX_LIST_COPY_STR(...) rox_list_create_str(NULL, __VA_ARGS__, NULL)

#define ROX_SET(...) rox_set_create(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_SET ROX_SET(NULL)

#define ROX_COPY(str) mem_copy_str(str)

#define ROX_MAP(...) rox_map_create(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_MAP ROX_MAP(NULL)

/**
 * Collectc has a bug in HASHSET_FOREACH it uses HashsetIter name instead of HashSetIter.
 */
#define ROX_SET_FOREACH(val, hashset, body)                             \
    {                                                                   \
        HashSetIter hashset_iter_53d46d2a04458e7b;                      \
        hashset_iter_init(&hashset_iter_53d46d2a04458e7b, hashset);     \
        void *val;                                                      \
        while (hashset_iter_next(&hashset_iter_53d46d2a04458e7b, &val) != CC_ITER_END) \
            body                                                        \
                }