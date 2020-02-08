#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

#include "util.h"
#include "base64.h"
#include "strrep.h"
#include "md5.h"

void *ROX_INTERNAL mem_copy(void *ptr, size_t bytes) {
    assert(ptr);
    assert(bytes >= 0);
    unsigned char *copy = malloc(bytes);
    memcpy(copy, ptr, bytes);
    return copy;
}

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

bool *ROX_INTERNAL mem_copy_bool(bool value) {
    bool *copy = malloc(sizeof(bool));
    *copy = value;
    return copy;
}

HashTable *ROX_INTERNAL mem_copy_map(HashTable *map) {
    assert(map);
    HashTable *params;
    hashtable_new(&params);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        hashtable_add(params, entry->key, entry->value);
    })
    return params;
}

HashTable *ROX_INTERNAL mem_deep_copy_str_value_map(HashTable *map) {
    assert(map);
    HashTable *copy;
    hashtable_new(&copy);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        hashtable_add(copy, entry->key, mem_copy_str(entry->value));
    })
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

#undef ROX_MEM_INT_TO_STR_BUFFER_SIZE

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

#undef MEM_DOUBLE_TO_STR_BUFFER_SIZE

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

#undef ROX_STR_MATCHES_BUFFER_SIZE

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

char *ROX_INTERNAL str_to_upper(char *str) {
    assert(str);
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = (char) toupper(str[i]);
    }
    return str;
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

size_t ROX_INTERNAL str_copy_value_to_buffer(char *buffer, int buffer_size, const char *value) {
    assert(buffer);
    assert(buffer_size > 0);
    assert(value);
    size_t len = strlen(value);
    assert(len < buffer_size);
    strncpy_s(buffer, buffer_size, value, len);
    return len;
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
    size_t len = strlen(s1) + strlen(s2) + 1;
    char *buffer = calloc(len, sizeof(char));
    sprintf_s(buffer, len, "%s%s", s1, s2);
    return buffer;
}

#define ROX_MEM_STR_FORMAT_BUFFER_SIZE 2048

char *ROX_INTERNAL mem_str_format(const char *fmt, ...) {
    assert(fmt);
    char buffer[ROX_MEM_STR_FORMAT_BUFFER_SIZE];
    va_list args;
            va_start(args, fmt);
    vsprintf_s(buffer, ROX_MEM_STR_FORMAT_BUFFER_SIZE, fmt, args);
            va_end(args);
    return mem_copy_str(buffer);
}

#undef ROX_MEM_STR_FORMAT_BUFFER_SIZE

double ROX_INTERNAL current_time_millis() {
    time_t t;
    time(&t);
    // TODO: get millis somehow
    return (double) (t * 1000);
}

size_t ROX_INTERNAL rox_file_read_b(const char *file_path, unsigned char *buffer, size_t buffer_size) {
    FILE *fp;
    if ((fopen_s(&fp, file_path, "rb")) != 0 || !fp) {
        return -1;
    }
    if (fseek(fp, 0, SEEK_END)) {
        fclose(fp);
        return -1;
    }
    const size_t file_size = ftell(fp);
    if (file_size == -1) {
        fclose(fp);
        return -1;
    }
    if ((size_t) file_size > buffer_size) {
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0, SEEK_SET)) {
        fclose(fp);
        return -1;
    }
    if (fread(buffer, sizeof(char), file_size, fp) != file_size) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return file_size;
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

#undef ROX_MEM_BASE64_ENCODE_BUFFER_SIZE

void ROX_INTERNAL md5_str_b(const char *s, unsigned char *buffer) {
    MD5_CTX context;
    size_t len = strlen(s);
    MD5_Init(&context);
    MD5_Update(&context, s, len);
    MD5_Final(buffer, &context);
}

char *ROX_INTERNAL mem_md5_str(const char *s) {
    assert(s);
    unsigned char digest[16];
    md5_str_b(s, digest);
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

unsigned char *ROX_INTERNAL mem_sha256(const char *s) {
    assert(s);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s, strlen(s));
    SHA256_Final(hash, &sha256);
    return mem_copy(hash, SHA256_DIGEST_LENGTH);
}

char *ROX_INTERNAL mem_sha256_str(const char *s) {
    assert(s);
    unsigned char *hash = mem_sha256(s);
    int i = 0;
    char *result = malloc(65);
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(result + (i * 2), "%02x", hash[i]);
    }
    result[64] = 0;
    free(hash);
    return result;
}

char *ROX_INTERNAL mem_str_join(const char *separator, List *strings) {
    assert(separator);
    assert(strings);
    size_t result_len = 0;
    int count = 0;
    LIST_FOREACH(item, strings, {
        char *str = (char *) item;
        result_len += strlen(str);
        ++count;
    })
    if (count == 0) {
        return mem_copy_str("");
    }
    size_t separator_len = strlen(separator);
    result_len += separator_len * (count - 1);
    char *result = malloc((result_len + 1) * sizeof(char));
    result[result_len] = '\0';
    char *dest = result;
    LIST_FOREACH(item, strings, {
        char *str = (char *) item;
        int len = strlen(str);
        if (len > 0) {
            strncpy_s(dest, len + 1, str, len);
            dest += len;
        }
        if (--count > 0) {
            if (separator_len > 0) {
                strncpy_s(dest, separator_len + 1, separator, separator_len);
                dest += separator_len;
            }
        }
    })
    return result;
}

/**
 * @param s The BASE64-ed string to decode.
 * @return Size if the resulting decoded data in bytes.
 */
size_t ROX_INTERNAL base64_decode_b(const char *s, unsigned char *buffer) {
    assert(s);
    assert(buffer);
    size_t len = strlen(s);
    size_t result_len;
    int result = base64decode(s, len, buffer, &result_len);
    assert(result == 0);
    return result_len;
}

#define ROX_MEM_BASE64_DECODE_BUFFER_SIZE 1024

unsigned char *ROX_INTERNAL mem_base64_decode(const char *s, size_t *result_length) {
    assert(s);
    unsigned char buffer[ROX_MEM_BASE64_DECODE_BUFFER_SIZE];
    size_t len = strlen(s);
    size_t resulting_str_len;
    int result = base64decode(s, len, buffer, &resulting_str_len);
    assert(result == 0);
    if (result == 0) {
        unsigned char *copy = mem_copy(buffer, resulting_str_len);
        if (result_length) {
            *result_length = resulting_str_len;
        }
        return copy;
    }
    return NULL;
}

char *ROX_INTERNAL mem_base64_decode_str(const char *s) {
    assert(s);
    unsigned char buffer[ROX_MEM_BASE64_DECODE_BUFFER_SIZE];
    size_t resulting_str_len = base64_decode_b(s, buffer);
    assert(resulting_str_len);
    assert(resulting_str_len < ROX_MEM_BASE64_DECODE_BUFFER_SIZE);
    buffer[resulting_str_len] = 0;
    return mem_copy(buffer, resulting_str_len + 1);
}

#undef ROX_MEM_BASE64_DECODE_BUFFER_SIZE

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

HashSet *ROX_INTERNAL rox_set_create(void *skip, ...) {
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

HashTable *rox_map_create(void *skip, ...) {
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

void ROX_INTERNAL rox_map_free_with_values(HashTable *map) {
    assert(map);
    rox_map_free_with_values_cb(map, &free);
}

void ROX_INTERNAL rox_map_free_with_keys_and_values(HashTable *map) {
    assert(map);
    rox_hash_table_free_with_keys_and_values_cb(map, &free, &free);
}

void ROX_INTERNAL rox_map_free_with_values_cb(HashTable *map, void (*f)(void *)) {
    assert(map);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        f(entry->value);
    })
    hashtable_destroy(map);
}

void ROX_INTERNAL
rox_hash_table_free_with_keys_and_values_cb(HashTable *map, void (*f_key)(void *), void (*f_value)(void *)) {
    assert(map);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        f_key(entry->key);
        if (entry->value) {
            f_value(entry->value);
        }
    })
    hashtable_destroy(map);
}