#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

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

#define ROX_MEM_INT_TO_STR_BUFFER_SIZE 10

char *ROX_INTERNAL mem_int_to_str(int value) {
    char buffer[ROX_MEM_INT_TO_STR_BUFFER_SIZE];
    _itoa_s(value, buffer, ROX_MEM_INT_TO_STR_BUFFER_SIZE, 10);
    return mem_copy_str(buffer);
}

#define MEM_DOUBLE_TO_STR_BUFFER_SIZE 100

char *ROX_INTERNAL mem_double_to_str(double value) {
    char buffer[MEM_DOUBLE_TO_STR_BUFFER_SIZE];
    int len = sprintf_s(buffer, MEM_DOUBLE_TO_STR_BUFFER_SIZE, "%f", value);
    // trim trailing zeroes
    for (int i = len - 1; i > 0; --i) {
        if (buffer[i] == '0' || buffer[i] == '.') {
            buffer[i] = '\0';
        } else {
            break;
        }
    }
    return mem_copy_str(buffer);
}

char *ROX_INTERNAL mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value) {
    return value
           ? mem_copy_str(true_value)
           : mem_copy_str(false_value);
}

#define ROX_STR_MATCHES_BUFFER_SIZE 256

bool ROX_INTERNAL str_matches(const char *str, const char *pattern, unsigned int options) {

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
        PCRE2_UCHAR buffer[ROX_STR_MATCHES_BUFFER_SIZE];
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

bool ROX_INTERNAL str_is_empty(const char *str) {
    return !str || str_equals(str, "");
}

bool ROX_INTERNAL str_in_list(const char *str, List *list_of_strings) {
    assert(str);
    assert(list_of_strings);
    return list_contains_value(list_of_strings,
                               (void *) str,
                               (int (*)(const void *, const void *)) &strcmp);
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

void ROX_INTERNAL str_copy_value_to_buffer(char *buffer, int buffer_size, const char *value) {
    assert(buffer);
    assert(buffer_size > 0);
    assert(value);
    size_t len = strlen(value);
    assert(len < buffer_size);
    strncpy_s(buffer, buffer_size, value, len);
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
    char *replaced = strrep(str, search, rep);
    return replaced == str // Pointer to the same string, not modified
           ? mem_copy_str(str)
           : replaced;
}

char *ROX_INTERNAL mem_str_concat(const char *s1, const char *s2) {
    assert(s1);
    assert(s2);
    size_t len = strlen(s1) + strlen(s2);
    char *buffer = calloc(len + 1, sizeof(char));
    sprintf_s(buffer, len, "%s%s", s1, s2);
    return buffer;
}

#define ROX_MEM_STR_FORMAT_BUFFER_SIZE 1024

char *ROX_INTERNAL mem_str_format(const char *fmt, ...) {
    assert(fmt);
    char buffer[ROX_MEM_STR_FORMAT_BUFFER_SIZE];
    va_list args;
            va_start(args, fmt);
    vsprintf_s(buffer, ROX_MEM_STR_FORMAT_BUFFER_SIZE, fmt, args);
    return mem_copy_str(buffer);
}

double ROX_INTERNAL current_time_millis() {
    time_t t;
    time(&t);
    // TODO: get millis somehow
    return (double) (t * 1000);
}

#define ROX_MEM_BASE64_ENCODE_BUFFER_SIZE 1024

char *ROX_INTERNAL mem_base64_encode(const char *s) {
    assert(s);
    char buffer[ROX_MEM_BASE64_ENCODE_BUFFER_SIZE];
    size_t len = strlen(s);
    int result = base64encode(s, len * sizeof(char), buffer, ROX_MEM_BASE64_ENCODE_BUFFER_SIZE);
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

#define ROX_MEM_BASE64_DECODE_BUFFER_SIZE 1024

char *ROX_INTERNAL mem_base64_decode(const char *s) {
    assert(s);
    unsigned char buffer[ROX_MEM_BASE64_DECODE_BUFFER_SIZE];
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

void ROX_INTERNAL rox_json_serialize(char *buffer, size_t buffer_size, unsigned int options, ...) {
    va_list args;
            va_start(args, options);

    cJSON *json = cJSON_CreateObject();
    char *property_name = va_arg(args, char*);
    while (property_name != NULL) {
        cJSON *property_value = va_arg(args, cJSON *);
        cJSON_AddItemToObject(json, property_name, property_value);
        property_name = va_arg(args, char*);
    };
            va_end(args);

    char *str = (options & ROX_JSON_PRETTY_PRINT) != 0
                ? cJSON_Print(json)
                : cJSON_PrintUnformatted(json);
    str_copy_value_to_buffer(buffer, buffer_size, str);
    cJSON_Delete(json);
    free(str);
}

List *ROX_INTERNAL rox_list_create(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    List *list;
    list_new(&list);
    void *item = va_arg(args, void*);
    while (item != NULL) {
        list_add(list, item);
        item = va_arg(args, void*);
    };
            va_end(args);
    return list;
}

List *ROX_INTERNAL rox_list_create_str(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    List *list;
    list_new(&list);
    char *item = va_arg(args, char*);
    while (item != NULL) {
        list_add(list, mem_copy_str(item));
        item = va_arg(args, char*);
    };
            va_end(args);
    return list;
}

HashSet *ROX_INTERNAL rox_hash_set_create(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    HashSet *hash_set;
    hashset_new(&hash_set);
    void *item = va_arg(args, void*);
    while (item != NULL) {
        hashset_add(hash_set, item);
        item = va_arg(args, void*);
    };
            va_end(args);
    return hash_set;
}

HashTable *rox_hash_table_create(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    HashTable *map;
    hashtable_new(&map);
    char *property_name = va_arg(args, char*);
    while (property_name != NULL) {
        void *property_value = va_arg(args, void *);
        hashtable_add(map, property_name, property_value);
        property_name = va_arg(args, char*);
    };
            va_end(args);
    return map;
}
