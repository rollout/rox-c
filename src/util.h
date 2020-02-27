#pragma once

#include <cjson/cJSON.h>
#include <collectc/list.h>
#include <collectc/hashset.h>
#include <collectc/hashtable.h>
#include <stdbool.h>
#include <time.h>
#include "rollout.h"

//
// Utility functions.
//
// NOTE: all functions prefixed with 'mem_' allocate memory
// and so returned pointer must be freed by the caller.
//
// On the contrary, all functions having '_b' suffix require buffer
// to be pre-allocated and passed as the (usually last) argument.
//

ROX_INTERNAL void *mem_copy(void *ptr, size_t bytes);

ROX_INTERNAL int *mem_copy_int(int value);

ROX_INTERNAL double *mem_copy_double(double value);

ROX_INTERNAL char *mem_copy_str(const char *ptr);

ROX_INTERNAL bool *mem_copy_bool(bool value);

ROX_INTERNAL HashTable *mem_copy_map(HashTable *map);

ROX_INTERNAL List *mem_copy_list(List *list);

ROX_INTERNAL List *mem_deep_copy_list(List *list, void *(*copy_func)(void *));

ROX_INTERNAL HashSet *mem_copy_set(HashSet *set);

ROX_INTERNAL HashSet *mem_deep_copy_set(HashSet *set, void *(*copy_func)(void *));

ROX_INTERNAL HashTable *mem_deep_copy_str_value_map(HashTable *map);

ROX_INTERNAL int *mem_str_to_int(const char *str);

ROX_INTERNAL double *mem_str_to_double(const char *str);

ROX_INTERNAL char *mem_int_to_str(int value);

ROX_INTERNAL char *mem_double_to_str(double value);

ROX_INTERNAL char *mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value);

ROX_INTERNAL bool str_matches(const char *str, const char *pattern, unsigned int options);

ROX_INTERNAL int str_index_of(const char *str, char c);

ROX_INTERNAL bool str_starts_with(const char *str, const char *prefix);

ROX_INTERNAL bool str_equals(const char *str, const char *another);

ROX_INTERNAL bool str_eq_n(const char *str, int start, int end, const char *another);

ROX_INTERNAL bool str_is_empty(const char *str);

/**
 * Note the passed <code>str</code> is modified in-place, without creating new strings.
 *
 * @param str Not <code>NULL</code>.
 * @return <code>str</code> itself, no new string is created.
 */
ROX_INTERNAL char *str_to_upper(char *str);

ROX_INTERNAL bool str_in_list(const char *str, List *list_of_strings);

ROX_INTERNAL void str_substring_b(const char *str, int start, int len, char *buffer);

ROX_INTERNAL size_t str_copy_value_to_buffer(char *buffer, size_t buffer_size, const char *value);

ROX_INTERNAL char *str_format_b(char *buffer, size_t buffer_size, const char *fmt, ...);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string. <em>Must</em> be zero-terminated.
 * @param start The start offset.
 * @param len Length of the returned substring.
 * @return Pointer to the NEWLY CREATED string which is a substring of the given string or NULL in case where start offset or length is out of bounds.
 */
ROX_INTERNAL char *mem_str_substring(const char *str, int start, int len);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string. Could be non-zero-terminated.
 * @param str_len The length of the given <code>str</code>.
 * @param start The start offset.
 * @param len Length of the returned substring.
 * @return Pointer to the NEWLY CREATED string which is a substring of the given string or NULL in case where start offset or length is out of bounds.
 */
ROX_INTERNAL char *mem_str_substring_n(const char *str, size_t str_len, int start, int len);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param str The input string.
 * @param search The string to replace.
 * @param rep The replacement.
 * @return Pointer to the NEWLY CREATED string in which all occurrences of <code>search</code> are replaced with <code>rep</code>.
 */
ROX_INTERNAL char *mem_str_replace(const char *str, const char *search, const char *rep);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s1 The left part. MUST NOT BE NULL.
 * @param s2 The right part. MUST NOT BE NULL.
 * @return Pointer to the NEWLY CREATED string which is a concatenation of the given two strings.
 */
ROX_INTERNAL char *mem_str_concat(const char *s1, const char *s2);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param fmt Format string. MUST NOT BE NULL.
 * @return Pointer to the NEWLY CREATED string which is a formatted string.
 */
ROX_INTERNAL char *mem_str_format(const char *fmt, ...);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @return Pointer to the NEWLY CREATED string which is a formatted string.
 */
ROX_INTERNAL char *mem_build_url(const char *base_uri, const char *path);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The string to encode.
 * @return Pointer to the NEWLY CREATED string which is a base64-encoded version of the given string.
 */
ROX_INTERNAL char *mem_base64_encode(const char *s);

/**
 * @param s The BASE64-ed string to decode.
 * @return Size if the resulting decoded data in bytes.
 */
ROX_INTERNAL size_t base64_decode_b(const char *s, unsigned char *buffer);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The BASE64-ed string to decode.
 * @return Pointer to the NEWLY CREATED string which is a base64-decoded version of the given string.
 */
ROX_INTERNAL char *mem_base64_decode_str(const char *s);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The input string. Not <code>NULL</code>.
 * @param buffer Buffer to output result to. Must not be <code>NULL</code>. Must be of length 16 (at least).
 */
ROX_INTERNAL void md5_str_b(const char *s, unsigned char *buffer);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param s The input string. Not <code>NULL</code>.
 * @return Pointer to the NEWLY CREATED string which is a binary md5 hash of the given string.
 */
ROX_INTERNAL char *mem_md5_str(const char *s);

ROX_INTERNAL unsigned char *mem_sha256(const char *s);

ROX_INTERNAL char *mem_sha256_str(const char *s);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param separator Separator string. Not <code>NULL</code>.
 * @param strings List of <code>char *</code>. Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *mem_str_join(const char *separator, List *strings);

/**
 * @return Number of milliseconds since Unix Epoch.
 */
ROX_INTERNAL double current_time_millis();

ROX_INTERNAL void thread_sleep(int sleep_millis);

ROX_INTERNAL struct timespec get_current_timespec();

ROX_INTERNAL struct timespec get_future_timespec(int ms);

/**
 * @param file_path Path to the file to read. Not <code>NULL</code>.
 * @param buffer Not <code>NULL</code>.
 * @param buffer_size Not negative.
 * @return Number of bytes read or -1 in case of an error.
 */
ROX_INTERNAL size_t rox_file_read_b(const char *file_path, unsigned char *buffer, size_t buffer_size);

ROX_INTERNAL cJSON *rox_json_create_object(void *skip, ...);

ROX_INTERNAL cJSON *rox_json_create_array(void *skip, ...);

ROX_INTERNAL List *rox_list_create(void *skip, ...);

ROX_INTERNAL List *rox_list_create_str(void *skip, ...);

ROX_INTERNAL bool list_equals(List *one, List *another, bool (*cmp)(void *v1, void *v2));

ROX_INTERNAL bool str_list_equals(List *one, List *another);

ROX_INTERNAL HashSet *rox_set_create(void *skip, ...);

ROX_INTERNAL HashTable *rox_map_create(void *skip, ...);

ROX_INTERNAL void rox_map_free_with_values(HashTable *map);

ROX_INTERNAL void rox_map_free_with_values_cb(HashTable *map, void (*f)(void *));

ROX_INTERNAL void rox_map_free_with_keys_and_values(HashTable *map);

ROX_INTERNAL void rox_hash_table_free_with_keys_and_values_cb(
        HashTable *map, void (*f_key)(void *), void (*f_value)(void *));

#define ROX_JSON_PRINT_FORMATTED 1u

ROX_INTERNAL char *rox_json_print(cJSON *json, unsigned int flags);

#define ROX_JSON_SERIALIZE(json) rox_json_print(json, 0)

#define ROX_JSON_SERIALIZE_PRETTY(json) rox_json_print(json, ROX_JSON_PRINT_FORMATTED)

#define ROX_JSON_OBJECT(...) rox_json_create_object(NULL, __VA_ARGS__, NULL)

#define ROX_JSON_ARRAY(...) rox_json_create_array(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_JSON_ARRAY ROX_JSON_ARRAY(NULL)

#define ROX_JSON_STRING(value) cJSON_CreateString(value)

#define ROX_JSON_INT(value) cJSON_CreateNumber(value)

#define ROX_JSON_DOUBLE(value) cJSON_CreateNumber(value)

#define ROX_JSON_TRUE cJSON_CreateTrue()

#define ROX_JSON_FALSE cJSON_CreateFalse()

#define ROX_JSON_BOOL cJSON_CreateBool()

#define ROX_JSON_NULL cJSON_CreateNull()

#define ROX_LIST(...) rox_list_create(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_LIST ROX_LIST(NULL)

#define ROX_LIST_COPY_STR(...) rox_list_create_str(NULL, __VA_ARGS__, NULL)

#define ROX_SET(...) rox_set_create(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_SET ROX_SET(NULL)

#define ROX_COPY(str) mem_copy_str(str)

#define ROX_MAP(...) rox_map_create(NULL, __VA_ARGS__, NULL)

#define ROX_STR(value) #value

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