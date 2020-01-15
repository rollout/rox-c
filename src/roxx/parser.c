#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pcre2.h>

#include "roxapi.h"
#include "util.h"
#include "core/entities.h"
#include "roxx/parser.h"
#include "roxx/stack.h"

//
// EvaluationResult
//

struct ROX_INTERNAL EvaluationResult {
    int *int_value;
    double *double_value;
    char *str_value;
    bool *bool_value;
};

EvaluationResult *ROX_INTERNAL _create_result_from_stack_item(StackItem *item) {
    assert(item);

    EvaluationResult *result = calloc(1, sizeof(EvaluationResult));

    if (stack_is_null(item)) {
        result->bool_value = false;
    }

    if (stack_is_boolean(item)) {
        result->bool_value = malloc(sizeof(bool));
        *result->bool_value = stack_get_boolean(item);
        result->str_value = mem_copy_str(*result->bool_value
                                         ? FLAG_TRUE_VALUE
                                         : FLAG_FALSE_VALUE);
    }

    if (stack_is_int(item)) {
        int value = stack_get_int(item);
        result->int_value = mem_copy_int(value);
        result->double_value = mem_copy_double(value);
    }

    if (stack_is_double(item)) {
        double value = stack_get_double(item);
        result->int_value = mem_copy_int((int) value);
        result->double_value = mem_copy_double(value);
    }

    if (stack_is_string(item)) {
        char *value = stack_get_string(item);
        result->int_value = str_to_int(value);
        result->double_value = str_to_double(value);
    }

    return result;
}

int *ROX_INTERNAL result_get_int(EvaluationResult *result) {
    return result->int_value;
}

double *ROX_INTERNAL result_get_double(EvaluationResult *result) {
    assert(result);
    return result->double_value;
}

bool *ROX_INTERNAL result_get_boolean(EvaluationResult *result) {
    assert(result);
    return result->bool_value;
}

char *ROX_INTERNAL result_get_string(EvaluationResult *result) {
    assert(result);
    return result->str_value;
}

void ROX_INTERNAL result_free(EvaluationResult *result) {
    assert(result);
    if (result->int_value) {
        free(result->int_value);
    }
    if (result->double_value) {
        free(result->double_value);
    }
    if (result->bool_value) {
        free(result->bool_value);
    }
    if (result->str_value) {
        free(result->str_value);
    }
    free(result);
}

//
// Node
//

typedef enum ROX_INTERNAL {
    Rand,
    Rator,
    Unknown
} NodeType;

typedef struct {
    NodeType type;
    // TODO: value (string, array, map, boolean, UNDEFINED, double, null)
} Node;

//
// Symbols
//

static const char *ROXX_UNDEFINED = "undefined";
static const char *ROXX_TRUE = "true";
static const char *ROXX_FALSE = "false";
static const char *ROXX_EMPTY_STRING = "\"\"";
static const char *ROXX_STRING_TYPE = "StringType";
static const char *ROXX_BOOL_TYPE = "BooleanType";
static const char *ROXX_NUMBER_TYPE = "NumberType";
static const char *ROXX_UNDEFINED_TYPE = "UndefinedType";
static const char *ROXX_NOT_A_TYPE = "NOT_A_TYPE";

//
// TokenTypes
//

const char *ROX_INTERNAL _get_type_from_token(const char *token) {
    if (!token) {
        return ROXX_NOT_A_TYPE;
    }

    if (str_matches(token, "\"((\\\\.)|[^\\\\\\\\\"])*\"", PCRE2_CASELESS)) {
        return ROXX_STRING_TYPE;
    };

    if (str_matches(token, "[\\-]{0,1}\\d+[\\.]\\d+|[\\-]{0,1}\\d+", PCRE2_CASELESS)) {
        return ROXX_NUMBER_TYPE;
    };

    if (str_matches(token, "true|false", PCRE2_CASELESS)) {
        return ROXX_BOOL_TYPE;
    };

    if (str_matches(token, ROXX_UNDEFINED, PCRE2_CASELESS)) {
        return ROXX_UNDEFINED_TYPE;
    };

    return ROXX_NOT_A_TYPE;
}

//
// StringTokenizer.
// C port of Java's StringTokenizer.
//

typedef struct ROX_INTERNAL StringTokenizer {
    int current_position;
    int new_position;
    int max_position;
    const char *str;
    const char *delimiters;
    bool ret_delims;
    bool delims_changed;
    int max_delim_code_point;
    int *delimiter_code_points;
    int delimiter_code_points_length;
} StringTokenizer;

static const int MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;

int ROX_INTERNAL _tokenizer_char_count(int codePoint) {
    return codePoint >= MIN_SUPPLEMENTARY_CODE_POINT ? 2 : 1;
}

void ROX_INTERNAL _tokenizer_set_max_delim_code_point(StringTokenizer *tokenizer) {
    assert(tokenizer);

    if (tokenizer->delimiters == NULL) {
        tokenizer->max_delim_code_point = 0;
        return;
    }

    int m = 0;
    int c;
    int count = 0;
    for (int i = 0; i < strlen(tokenizer->delimiters); i += _tokenizer_char_count(c)) {
        c = tokenizer->delimiters[i];
        if (m < c)
            m = c;
        count++;
    }
    tokenizer->max_delim_code_point = m;
}

StringTokenizer *ROX_INTERNAL tokenizer_create(const char *str, const char *delim, bool return_delims) {
    assert(str);
    assert(delim);

    StringTokenizer *tokenizer = calloc(1, sizeof(StringTokenizer));
    tokenizer->current_position = 0;
    tokenizer->new_position = -1;
    tokenizer->delims_changed = false;
    tokenizer->str = str;
    tokenizer->max_position = (int) strlen(str);
    tokenizer->delimiters = delim;
    tokenizer->ret_delims = return_delims;

    _tokenizer_set_max_delim_code_point(tokenizer);

    return tokenizer;
}

StringTokenizer *ROX_INTERNAL tokenizer_create_default(const char *str) {
    assert(str);
    return tokenizer_create(str, " \t\n\r\f", false);
}

void ROX_INTERNAL tokenizer_free(StringTokenizer *tokenizer) {
    assert(tokenizer);
    // TODO: implement
    free(tokenizer);
}

bool ROX_INTERNAL _tokenizer_is_delimiter(StringTokenizer *tokenizer, int codePoint) {
    assert(tokenizer);
    for (int i = 0; i < tokenizer->delimiter_code_points_length; i++) {
        if (tokenizer->delimiter_code_points[i] == codePoint) {
            return true;
        }
    }
    return false;
}

int ROX_INTERNAL _tokenizer_skip_delimiters(StringTokenizer *tokenizer, int star_pos) {
    assert(tokenizer);
    assert(tokenizer->delimiters);

    int position = star_pos;
    while (!tokenizer->ret_delims && position < tokenizer->max_position) {
        char c = tokenizer->str[position];
        if ((c > tokenizer->max_delim_code_point) || (str_index_of(tokenizer->delimiters, c) < 0))
            break;
        position++;
    }
    return position;
}

int ROX_INTERNAL _tokenizer_scan_token(StringTokenizer *tokenizer, int startPos) {
    assert(tokenizer);
    int position = startPos;
    while (position < tokenizer->max_position) {
        char c = tokenizer->str[position];
        if ((c <= tokenizer->max_delim_code_point) && (str_index_of(tokenizer->delimiters, c) >= 0))
            break;
        position++;
    }
    if (tokenizer->ret_delims && (startPos == position)) {
        char c = tokenizer->str[position];
        if ((c <= tokenizer->max_delim_code_point) && (str_index_of(tokenizer->delimiters, c) >= 0))
            position++;
    }
    return position;
}

bool ROX_INTERNAL tokenizer_has_more_tokens(StringTokenizer *tokenizer) {
    assert(tokenizer);
    tokenizer->new_position = _tokenizer_skip_delimiters(tokenizer, tokenizer->current_position);
    return (tokenizer->new_position < tokenizer->max_position);
}

/**
 * NOTE: THE RETURNED POINTER MUST BE FREED AFTER USAGE.
 */
char *ROX_INTERNAL tokenizer_next_token(StringTokenizer *tokenizer) {
    assert(tokenizer);
    tokenizer->current_position = (tokenizer->new_position >= 0 && !tokenizer->delims_changed) ?
                                  tokenizer->new_position : _tokenizer_skip_delimiters(tokenizer,
                                                                                       tokenizer->current_position);

    /* Reset these anyway */
    tokenizer->delims_changed = false;
    tokenizer->new_position = -1;

    if (tokenizer->current_position >= tokenizer->max_position) {
        // TODO: log
        fprintf(stderr, "tokenizer->current_position (%d) is "
                        "expected to be greater or equal to tokenizer->max_position (%d)",
                tokenizer->current_position,
                tokenizer->max_position);
        exit(1);
    }

    int start = tokenizer->current_position;
    tokenizer->current_position = _tokenizer_scan_token(tokenizer, tokenizer->current_position);
    return str_substring(tokenizer->str, start, (tokenizer->current_position - start));
}
