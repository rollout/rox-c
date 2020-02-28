#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "util.h"
#include "vendor/base64.h"
#include "vendor/strrep.h"
#include "vendor/md5.h"
#include "os.h"
#include "core/logging.h"

#ifdef ROX_WINDOWS

#include <windows.h>

#else
#include <unistd.h>
#endif

#ifdef ROX_APPLE
#include <sys/time.h>
#else

#include <time.h>

#endif

ROX_INTERNAL void *mem_copy(void *ptr, size_t bytes) {
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

ROX_API char *mem_copy_str(const char *ptr) {
    assert(ptr);
    size_t length = strlen(ptr);
    char *copy = malloc((length + 1) * sizeof(char));
    strncpy(copy, ptr, length + 1);
    return copy;
}

ROX_INTERNAL bool *mem_copy_bool(bool value) {
    bool *copy = malloc(sizeof(bool));
    *copy = value;
    return copy;
}

ROX_INTERNAL int *mem_str_to_int(const char *str) {
    assert(str);
    long num = strtol(str, NULL, 0);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_int(num);
}

ROX_INTERNAL double *mem_str_to_double(const char *str) {
    assert(str);
    double num = strtod(str, NULL);
    if (num == 0 && str[0] != '0') {
        return NULL;
    }
    return mem_copy_double(num);
}

#define ROX_MEM_INT_TO_STR_BUFFER_SIZE 10

ROX_INTERNAL char *mem_int_to_str(int value) {
    char buffer[ROX_MEM_INT_TO_STR_BUFFER_SIZE];
    snprintf(buffer, ROX_MEM_INT_TO_STR_BUFFER_SIZE, "%d", value);
    return mem_copy_str(buffer);
}

#undef ROX_MEM_INT_TO_STR_BUFFER_SIZE

#define MEM_DOUBLE_TO_STR_BUFFER_SIZE 100

ROX_INTERNAL char *mem_double_to_str(double value) {
    char buffer[MEM_DOUBLE_TO_STR_BUFFER_SIZE];
    int len = snprintf(buffer, MEM_DOUBLE_TO_STR_BUFFER_SIZE, "%f", value);
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

ROX_INTERNAL char *mem_bool_to_str(bool value,
                                   const char *true_value,
                                   const char *false_value) {
    return value
           ? mem_copy_str(true_value)
           : mem_copy_str(false_value);
}

#define ROX_STR_MATCHES_BUFFER_SIZE 256

ROX_INTERNAL bool str_matches(const char *str, const char *pattern, unsigned int options) {

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
        ROX_WARN("PCRE2 compilation failed at offset %d: %s", (int) error_offset, buffer);
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

ROX_INTERNAL int str_index_of(const char *str, char c) {
    assert(str);
    char *e = strchr(str, c);
    if (!e) {
        return -1;
    }
    return (int) (e - str);
}

ROX_INTERNAL bool str_starts_with(const char *str, const char *prefix) {
    assert(str);
    assert(prefix);
    for (int i = 0, n = (int) strlen(prefix); i < n; ++i) {
        if (str[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

ROX_INTERNAL bool str_equals(const char *str, const char *another) {
    assert(str);
    assert(another);
    return str == another || strcmp(str, another) == 0;
}

ROX_INTERNAL bool str_eq_n(const char *str, int start, int end, const char *another) {
    assert(str);
    assert(another);
    for (int i = start; i < end; ++i) {
        if (str[i] != another[i - start]) {
            return false;
        }
    }
    return true;
}

ROX_INTERNAL bool str_is_empty(const char *str) {
    return !str || str_equals(str, "");
}

ROX_INTERNAL char *str_to_upper(char *str) {
    assert(str);
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = (char) toupper(str[i]);
    }
    return str;
}

ROX_INTERNAL void str_substring_b(const char *str, int start, int len, char *buffer) {
    assert(str);
    assert(start >= 0);
    assert(len >= 0);
    assert(buffer);
    assert(start + len <= strlen(str));
    memcpy(buffer, str + start, len);
    buffer[len] = '\0';
}

ROX_INTERNAL size_t str_copy_value_to_buffer(char *buffer, size_t buffer_size, const char *value) {
    assert(buffer);
    assert(buffer_size > 0);
    assert(value);
    size_t len = strlen(value);
    assert(len < buffer_size);
    strncpy(buffer, value, len + 1);
    return len;
}

ROX_INTERNAL char *str_format_b(char *buffer, size_t buffer_size, const char *fmt, ...) {
    va_list args;
            va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
            va_end(args);
    return buffer;
}

ROX_INTERNAL char *mem_str_substring(const char *str, int start, int len) {
    assert(str);
    assert(start >= 0);
    assert(len >= 0);
    return mem_str_substring_n(str, strlen(str), start, len);
}

ROX_INTERNAL char *mem_str_substring_n(const char *str, size_t str_len, int start, int len) {
    assert(str);
    assert(start >= 0);
    assert(len >= 0);
    assert(str_len >= 0);
    if (start + len > str_len) {
        return NULL;
    }
    char *buffer = calloc(len + 1, sizeof(char));
    str_substring_b(str, start, len, buffer);
    return buffer;
}

ROX_INTERNAL char *mem_str_replace(const char *str, const char *search, const char *rep) {
    assert(str);
    assert(search);
    assert(rep);
    char *replaced = strrep(str, search, rep);
    char *result = replaced == str // Pointer to the same string, not modified
                   ? mem_copy_str(str)
                   : mem_copy_str(replaced);
    return result;
}

ROX_INTERNAL char *mem_str_concat(const char *s1, const char *s2) {
    assert(s1);
    assert(s2);
    size_t len = strlen(s1) + strlen(s2) + 1;
    char *buffer = calloc(len, sizeof(char));
    snprintf(buffer, len, "%s%s", s1, s2);
    return buffer;
}

#define ROX_MEM_STR_FORMAT_BUFFER_SIZE 2048

ROX_INTERNAL char *mem_str_format(const char *fmt, ...) {
    assert(fmt);
    char buffer[ROX_MEM_STR_FORMAT_BUFFER_SIZE];
    va_list args;
            va_start(args, fmt);
    vsnprintf(buffer, ROX_MEM_STR_FORMAT_BUFFER_SIZE, fmt, args);
            va_end(args);
    return mem_copy_str(buffer);
}

#undef ROX_MEM_STR_FORMAT_BUFFER_SIZE

ROX_INTERNAL char *mem_build_url(const char *base_uri, const char *path) {
    assert(base_uri);
    assert(path);
    size_t base_uri_len = strlen(base_uri);
    if (!base_uri_len) {
        return mem_copy_str("");
    }
    if (base_uri[base_uri_len - 1] == '/') {
        --base_uri_len;
    }
    size_t path_len = strlen(path);
    if (path_len > 0) {
        if (path[0] == '/') {
            ++path;
            --path_len;
        }
        size_t resulting_string_len = base_uri_len + 1 + path_len;
        char *result = malloc(resulting_string_len + 1);
        memcpy(result, base_uri, base_uri_len * sizeof(char));
        memcpy(result + base_uri_len, "/", sizeof(char));
        memcpy(result + base_uri_len + 1, path, path_len);
        result[resulting_string_len] = 0;
        return result;
    }
    return mem_copy_str(base_uri);
}

ROX_INTERNAL double current_time_millis() {
    time_t t;
    time(&t);
    // TODO: get millis somehow
    return (double) (t * 1000);
}

ROX_INTERNAL void thread_sleep(int sleep_millis) {
    assert(sleep_millis >= 0);
#ifdef ROX_WINDOWS
    Sleep(sleep_millis);
#else
    usleep(sleep_millis * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
}

ROX_INTERNAL struct timespec get_current_timespec() {
    struct timespec now;
#if defined(ROX_APPLE)
    int result = gettimeofday(&now, NULL);
    assert(result == 0);
#else
    int result = timespec_get(&now, TIME_UTC);
    assert(result != 0);
#endif
    return now;
}

ROX_INTERNAL struct timespec get_future_timespec(int ms) {
    struct timespec now = get_current_timespec(), due;
    due.tv_sec = now.tv_sec + ms / 1000;
    due.tv_nsec = now.tv_nsec + (ms % 1000) * 1000000;
    if (due.tv_nsec >= 1000000000) {
        due.tv_nsec -= 1000000000;
        due.tv_sec++;
    }
    return due;
}

ROX_INTERNAL size_t rox_file_read_b(const char *file_path, unsigned char *buffer, size_t buffer_size) {
    FILE *fp;
    if (!(fp = (fopen(file_path, "rb")))) {
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

// calculate the size of 'output' buffer required for a 'input' buffer of length x during Base64 encoding operation
#define B64ENCODE_OUT_SAFESIZE(x) ((((x) + 3 - 1)/3) * 4 + 1)

ROX_INTERNAL char *mem_base64_encode(const char *s) {
    assert(s);
    size_t len = strlen(s);
    size_t size = B64ENCODE_OUT_SAFESIZE(len);
    char *buffer = malloc(size);
    int result = base64encode(s, len * sizeof(char), buffer, size);
    if (result != 0) {
        ROX_DEBUG("Failed to decode str %s: %d", s, result);
        free(buffer);
        assert(result == 0);
        return NULL;
    }
    return buffer;
}

ROX_INTERNAL void md5_str_b(const char *s, unsigned char *buffer) {
    MD5_CTX context;
    size_t len = strlen(s);
    MD5_Init(&context);
    MD5_Update(&context, s, len);
    MD5_Final(buffer, &context);
}

ROX_INTERNAL char *mem_md5_str(const char *s) {
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

ROX_INTERNAL unsigned char *mem_sha256(const char *s) {
    assert(s);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s, strlen(s));
    SHA256_Final(hash, &sha256);
    return mem_copy(hash, SHA256_DIGEST_LENGTH);
}

ROX_INTERNAL char *mem_sha256_str(const char *s) {
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

ROX_INTERNAL size_t base64_decode_b(const char *s, unsigned char *buffer, size_t buffer_size) {
    assert(s);
    assert(buffer);
    size_t len = strlen(s);
    size_t result_len = buffer_size;
    int result = base64decode(s, len, buffer, &result_len);
    if (result != 0) {
        ROX_ERROR("Failed to decode str %s: %d", s, result);
    }
    assert(result == 0);
    return result_len;
}

// calculate the size of 'output' buffer required for a 'input' buffer of length x during Base64 decoding operation
#define B64DECODE_OUT_SAFESIZE(x) (((x)*3)/4)

ROX_INTERNAL unsigned char *mem_base64_decode(const char *s, size_t *result_length) {
    assert(s);
    size_t len = strlen(s);
    size_t size = B64DECODE_OUT_SAFESIZE(len) + 1;
    unsigned char *buffer = malloc(size);
    size_t resulting_str_len;
    int result = base64decode(s, len, buffer, &resulting_str_len);
    assert(result == 0);
    if (result == 0) {
        if (result_length) {
            *result_length = resulting_str_len;
        }
        return buffer;
    } else {
        free(buffer);
        return NULL;
    }
}

ROX_INTERNAL char *mem_base64_decode_str(const char *s) {
    assert(s);
    size_t size = B64DECODE_OUT_SAFESIZE(strlen(s)) + 1;
    unsigned char *buffer = malloc(size);
    size_t resulting_str_len = base64_decode_b(s, buffer, size);
    assert(resulting_str_len);
    assert(resulting_str_len < size);
    buffer[resulting_str_len] = 0;
    return buffer;
}

ROX_INTERNAL cJSON *rox_json_create_object(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    cJSON *json = cJSON_CreateObject();
    char *property_name = va_arg(args, char*);
    while (property_name) {
        cJSON *property_value = va_arg(args, cJSON *);
        cJSON_AddItemToObject(json, property_name, property_value);
        property_name = va_arg(args, char*);
    };
            va_end(args);

    return json;
}

ROX_INTERNAL cJSON *rox_json_create_array(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    cJSON *arr = cJSON_CreateArray();
    cJSON *item = va_arg(args, cJSON*);
    while (item) {
        cJSON_AddItemToArray(arr, item);
        item = va_arg(args, cJSON*);
    };
            va_end(args);
    return arr;
}

#define ROX_JSON_PRINT_BUFFER_SIZE 10240

ROX_INTERNAL char *rox_json_print(cJSON *json, unsigned int flags) {
    char buffer[ROX_JSON_PRINT_BUFFER_SIZE];
    cJSON_PrintPreallocated(json, buffer, ROX_JSON_PRINT_BUFFER_SIZE, (flags & ROX_JSON_PRINT_FORMATTED) != 0);
    return mem_copy_str(buffer);
}

#undef ROX_JSON_PRINT_BUFFER_SIZE