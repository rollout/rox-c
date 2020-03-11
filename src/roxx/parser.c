#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pcre2.h>
#include <float.h>
#include <math.h>

#include "rollout.h"
#include "util.h"
#include "parser.h"
#include "stack.h"
#include "vendor/semver.h"
#include "core/logging.h"
#include "collections.h"

//
// Symbols
//

ROX_INTERNAL const char *ROXX_UNDEFINED = "undefined";
ROX_INTERNAL const char *ROXX_TRUE = "true";
ROX_INTERNAL const char *ROXX_FALSE = "false";
ROX_INTERNAL const char *ROXX_EMPTY_STRING = "\"\"";

//
// TokenTypes
//

ROX_INTERNAL ParserTokenType get_token_type_from_token(const char *token) {
    if (!token) {
        return TokenTypeNotAType;
    }
    if (str_matches(token, "^\"((\\\\.)|[^\\\\\\\\\"])*\"$", PCRE2_CASELESS)) {
        return TokenTypeString;
    }
    if (str_matches(token, "^[\\-]{0,1}\\d+[\\.]\\d+|[\\-]{0,1}\\d+$", PCRE2_CASELESS)) {
        return TokenTypeNumber;
    }
    if (str_matches(token, "^true|false$", PCRE2_CASELESS)) {
        return TokenTypeBool;
    }
    if (str_matches(token, ROXX_UNDEFINED, PCRE2_CASELESS)) {
        return TokenTypeUndefined;
    }
    return TokenTypeNotAType;
}

//
// EvaluationResult
//

struct EvaluationResult {
    int *int_value;
    double *double_value;
    char *str_value;
    bool is_true;
    bool is_false;
    bool is_null;
};

ROX_INTERNAL EvaluationResult *_create_result_from_stack_item(StackItem *item) {
    EvaluationResult *result = calloc(1, sizeof(EvaluationResult));

    if (!item || rox_stack_is_null(item)) {
        result->is_null = true;
        return result;
    }

    if (rox_stack_is_undefined(item)) {
        return result;
    }

    if (rox_stack_is_boolean(item)) {
        bool value = rox_stack_get_boolean(item);
        result->is_true = value;
        result->is_false = !value;
        result->str_value = mem_copy_str(value ? ROXX_TRUE : ROXX_FALSE);
        return result;
    }

    if (rox_stack_is_numeric(item)) {
        int int_value = rox_stack_get_int(item);
        double double_value = rox_stack_get_double(item);
        result->int_value = mem_copy_int(int_value);
        result->double_value = mem_copy_double(double_value);
        result->str_value = mem_double_to_str(double_value);
        return result;
    }

    if (rox_stack_is_string(item)) {
        char *value = rox_stack_get_string(item);
        result->int_value = mem_str_to_int(value);
        result->double_value = mem_str_to_double(value);
        result->str_value = mem_copy_str(value);
        return result;
    }

    return result;
}

ROX_INTERNAL int *result_get_int(EvaluationResult *result) {
    return result->int_value;
}

ROX_INTERNAL double *result_get_double(EvaluationResult *result) {
    assert(result);
    return result->double_value;
}

ROX_INTERNAL bool *result_get_boolean(EvaluationResult *result) {
    assert(result);
    if (result->is_null) {
        return &result->is_true;
    }
    if (!result->is_true && !result->is_false) {
        return NULL;
    }
    return &result->is_true;
}

ROX_INTERNAL char *result_get_string(EvaluationResult *result) {
    assert(result);
    return result->str_value;
}

ROX_INTERNAL void result_free(EvaluationResult *result) {
    assert(result);
    if (result->int_value) {
        free(result->int_value);
    }
    if (result->double_value) {
        free(result->double_value);
    }
    if (result->str_value) {
        free(result->str_value);
    }
    free(result);
}

//
// Node
//

struct ParserNode {
    NodeType type;
    RoxDynamicValue *value;
};

ROX_INTERNAL NodeType node_get_type(ParserNode *node) {
    assert(node);
    return node->type;
}

ROX_INTERNAL RoxDynamicValue *node_get_value(ParserNode *node) {
    assert(node);
    return node->value;
}

ROX_INTERNAL ParserNode *_node_create_empty(NodeType type) {
    ParserNode *node = (ParserNode *) calloc(1, sizeof(ParserNode));
    node->type = type;
    return node;
}

ROX_INTERNAL ParserNode *node_create_str_ptr(NodeType type, char *str) {
    assert(str);
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_string_ptr(str);
    return node;
}

ROX_INTERNAL ParserNode *node_create_str_copy(NodeType type, const char *str) {
    assert(str);
    return node_create_str_ptr(type, mem_copy_str(str));
}

ROX_INTERNAL ParserNode *node_create_double_ptr(NodeType type, double *value) {
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_double_ptr(value);
    return node;
}

ROX_INTERNAL ParserNode *node_create_double(NodeType type, double value) {
    return node_create_double_ptr(type, mem_copy_double(value));
}

ROX_INTERNAL ParserNode *node_create_bool(NodeType type, bool value) {
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_boolean(value);
    return node;
}

ROX_INTERNAL ParserNode *node_create_list(NodeType type, RoxList *value) {
    assert(value);
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_list(value);
    return node;
}

ROX_INTERNAL ParserNode *node_create_map(NodeType type, RoxMap *value) {
    assert(value);
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_map(value);
    return node;
}

ROX_INTERNAL ParserNode *node_create_null(NodeType type) {
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_null();
    return node;
}

ROX_INTERNAL ParserNode *node_create_undefined(NodeType type) {
    ParserNode *node = _node_create_empty(type);
    node->value = rox_dynamic_value_create_undefined();
    return node;
}

ROX_INTERNAL void node_free(ParserNode *node) {
    assert(node);
    rox_dynamic_value_free(node->value);
    free(node);
}

//
// StringTokenizer.
// C port of Java's StringTokenizer.
//

typedef struct StringTokenizer {
    int current_position;
    int new_position;
    int max_position;
    char *str;
    char *delimiters;
    bool ret_delims;
    bool delims_changed;
    int max_delim_code_point;
} StringTokenizer;

ROX_INTERNAL void _tokenizer_set_max_delim_code_point(StringTokenizer *tokenizer) {
    assert(tokenizer);
    int m = 0;
    int c;
    int count = 0;
    for (int i = 0; i < strlen(tokenizer->delimiters); ++i) {
        c = tokenizer->delimiters[i]; // TODO: UTF8 support in expressions?
        if (m < c) {
            m = c;
        }
        count++;
    }
    tokenizer->max_delim_code_point = m;
}

ROX_INTERNAL StringTokenizer *tokenizer_create(const char *str, const char *delim, bool return_delims) {
    assert(str);
    assert(delim);

    StringTokenizer *tokenizer = calloc(1, sizeof(StringTokenizer));
    tokenizer->current_position = 0;
    tokenizer->new_position = -1;
    tokenizer->delims_changed = false;
    tokenizer->str = mem_copy_str(str);
    tokenizer->max_position = (int) strlen(str);
    tokenizer->delimiters = mem_copy_str(delim);
    tokenizer->ret_delims = return_delims;

    _tokenizer_set_max_delim_code_point(tokenizer);

    return tokenizer;
}

ROX_INTERNAL void tokenizer_free(StringTokenizer *tokenizer) {
    assert(tokenizer);
    free(tokenizer->delimiters);
    free(tokenizer->str);
    free(tokenizer);
}

ROX_INTERNAL int _tokenizer_skip_delimiters(StringTokenizer *tokenizer, int star_pos) {
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

ROX_INTERNAL int _tokenizer_scan_token(StringTokenizer *tokenizer, int startPos) {
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

ROX_INTERNAL bool tokenizer_has_more_tokens(StringTokenizer *tokenizer) {
    assert(tokenizer);
    tokenizer->new_position = _tokenizer_skip_delimiters(tokenizer, tokenizer->current_position);
    return (tokenizer->new_position < tokenizer->max_position);
}

/**
 * NOTE: THE RETURNED POINTER MUST BE FREED AFTER USAGE.
 */
ROX_INTERNAL void tokenizer_next_token(StringTokenizer *tokenizer, char *buffer, int *ret_len) {
    assert(tokenizer);
    tokenizer->current_position = (tokenizer->new_position >= 0 && !tokenizer->delims_changed) ?
                                  tokenizer->new_position : _tokenizer_skip_delimiters(tokenizer,
                                                                                       tokenizer->current_position);

    /* Reset these anyway */
    tokenizer->delims_changed = false;
    tokenizer->new_position = -1;

    if (tokenizer->current_position >= tokenizer->max_position) {
        ROX_ERROR("tokenizer->current_position (%d) is "
                  "expected to be greater or equal to tokenizer->max_position (%d)",
                  tokenizer->current_position,
                  tokenizer->max_position);
        exit(1);
    }

    int start = tokenizer->current_position;
    tokenizer->current_position = _tokenizer_scan_token(tokenizer, tokenizer->current_position);
    int len = tokenizer->current_position - start;
    str_substring_b(tokenizer->str, start, len, buffer);
    *ret_len = len;
}

ROX_INTERNAL void
tokenizer_next_token_with_delim(StringTokenizer *tokenizer, const char *delim, char *buffer, int *ret_len) {
    assert(tokenizer);
    assert(delim);
    assert(buffer);

    free(tokenizer->delimiters);
    tokenizer->delimiters = mem_copy_str(delim);

    /* delimiter string specified, so set the appropriate flag. */
    tokenizer->delims_changed = true;

    _tokenizer_set_max_delim_code_point(tokenizer);
    tokenizer_next_token(tokenizer, buffer, ret_len);
}

//
// TokenizedExpression
//

static const char *DICT_START_DELIMITER = "{";
static const char *DICT_END_DELIMITER = "}";
static const char *ARRAY_START_DELIMITER = "[";
static const char *ARRAY_END_DELIMITER = "]";
static const char *TOKEN_DELIMITERS = "{}[]():, \t\r\n\"";
static const char *PRE_POST_STRING_CHAR = "";
static const char *STRING_DELIMITER = "\"";
static const char *ESCAPED_QUOTE = "\\\"";
static const char *ESCAPED_QUOTE_PLACEHOLDER = "\\RO_Q";

typedef struct TokenizedExpression {
    RoxList *array_accumulator;
    RoxMap *dict_accumulator;
    char *dict_key;
} TokenizedExpression;

ROX_INTERNAL void tokenized_expression_free(TokenizedExpression *expr) {
    assert(expr);
    if (expr->array_accumulator) {
        ROX_WARN("Unclosed array literal in roxx expression");
        rox_list_free_cb(expr->array_accumulator,
                         (void (*)(void *)) &rox_dynamic_value_free);
    }
    if (expr->dict_accumulator) {
        ROX_WARN("Unclosed dictionary literal in roxx expression");
        rox_map_free_with_keys_and_values_cb(expr->dict_accumulator,
                                             &free, (void (*)(void *)) &rox_dynamic_value_free);
    }
    if (expr->dict_key) {
        ROX_WARN("Unclosed dictionary literal in roxx expression");
        free(expr->dict_key);
    }
    free(expr);
}

ROX_INTERNAL void _tokenized_expression_push_node(TokenizedExpression *expr, ParserNode *node, RoxList *node_list) {
    assert(expr);
    assert(node);
    assert(node_list);
    if (expr->dict_accumulator && !expr->dict_key) {
        expr->dict_key = mem_copy_str(rox_dynamic_value_get_string(node->value));
        node_free(node);
    } else if (expr->dict_accumulator && expr->dict_key) {
        if (!rox_map_contains_key(expr->dict_accumulator, expr->dict_key)) {
            rox_map_add(expr->dict_accumulator,
                        expr->dict_key,
                        rox_dynamic_value_create_copy(node->value));
        } else {
            free(expr->dict_key);
        }
        node_free(node);
        expr->dict_key = NULL;
    } else if (expr->array_accumulator) {
        rox_list_add(expr->array_accumulator, rox_dynamic_value_create_copy(node->value));
        node_free(node);
    } else {
        rox_list_add(node_list, node);
    }
}

ROX_INTERNAL ParserNode *_tokenized_expression_node_from_token(NodeType nodeType, const char *token) {
    assert(token);
    ParserTokenType token_type = get_token_type_from_token(token);
    if (str_equals(token, ROXX_TRUE)) {
        return node_create_bool(nodeType, true);
    }
    if (str_equals(token, ROXX_FALSE)) {
        return node_create_bool(nodeType, false);
    }
    if (str_equals(token, ROXX_UNDEFINED)) {
        return node_create_undefined(nodeType);
    }
    if (token_type == TokenTypeString) {
        int token_length = (int) strlen(token);
        char *value = mem_str_substring(token, 1, token_length - 2);
        return node_create_str_ptr(nodeType, value);
    }
    if (token_type == TokenTypeNumber) {
        double *value = mem_str_to_double(token);
        return node_create_double_ptr(nodeType, value);
    }
    return node_create_null(NodeTypeUnknown);
}

#define ROX_TOKEN_BUFFER_SIZE 1024

ROX_INTERNAL RoxList *tokenized_expression_get_tokens(const char *expression, RoxMap *operators) {
    assert(expression);
    assert(operators);

    TokenizedExpression *expr = calloc(1, sizeof(TokenizedExpression));

    RoxList *result_list = rox_list_create();

    const char *delimiters_to_use = TOKEN_DELIMITERS;
    char *normalized_expression = mem_str_replace(expression, ESCAPED_QUOTE, ESCAPED_QUOTE_PLACEHOLDER);
    StringTokenizer *tokenizer = tokenizer_create(normalized_expression, delimiters_to_use, true);
    free(normalized_expression);

    char prev_token[ROX_TOKEN_BUFFER_SIZE];
    char token[ROX_TOKEN_BUFFER_SIZE];
    int token_len = 0, prev_token_len = 0;
    prev_token[token_len] = token[token_len] = '\0';

    while (tokenizer_has_more_tokens(tokenizer)) {
        strncpy(prev_token, token, token_len + 1);
        prev_token_len = token_len;

        tokenizer_next_token_with_delim(tokenizer, delimiters_to_use, token, &token_len);
        bool in_string = str_equals(delimiters_to_use, STRING_DELIMITER);

        if (!in_string && str_equals(token, DICT_START_DELIMITER)) {
            if (expr->dict_accumulator) {
                ROX_WARN("new dict has started before the existing is closed");
                rox_map_free_with_keys_and_values_cb(
                        expr->dict_accumulator,
                        &free, (void (*)(void *)) &rox_dynamic_value_free); // FIXME: what about dict-in-dict case?
            }
            expr->dict_accumulator = rox_map_create();
        } else if (!in_string && str_equals(token, DICT_END_DELIMITER)) {
            RoxMap *dict_result = expr->dict_accumulator;
            expr->dict_accumulator = NULL;
            _tokenized_expression_push_node(
                    expr,
                    node_create_map(NodeTypeRand, dict_result),  // NOTE: dict_result must be freed in node_free
                    result_list);
        } else if (!in_string && str_equals(token, ARRAY_START_DELIMITER)) {
            if (expr->array_accumulator) {
                ROX_WARN("new array has started before the existing is closed");
                rox_list_free_cb(expr->array_accumulator,
                                 (void (*)(void *)) &rox_dynamic_value_free);  // FIXME: what about array-in-array case?
            }
            expr->array_accumulator = rox_list_create();
        } else if (!in_string && str_equals(token, ARRAY_END_DELIMITER)) {
            RoxList *array_result = expr->array_accumulator;
            expr->array_accumulator = NULL;
            _tokenized_expression_push_node(
                    expr,
                    node_create_list(NodeTypeRand, array_result), // NOTE: array_result must be freed in node_free
                    result_list);
        } else if (str_equals(token, STRING_DELIMITER)) {
            if (prev_token_len > 0 && str_equals(prev_token, STRING_DELIMITER)) {
                _tokenized_expression_push_node(
                        expr,
                        node_create_str_copy(NodeTypeRand, ROXX_EMPTY_STRING),
                        result_list);
            }
            delimiters_to_use = in_string
                                ? TOKEN_DELIMITERS
                                : STRING_DELIMITER;
        } else {
            if (str_equals(delimiters_to_use, STRING_DELIMITER)) {
                char *escaped_token = mem_str_replace(token, ESCAPED_QUOTE_PLACEHOLDER, ESCAPED_QUOTE);
                _tokenized_expression_push_node(
                        expr,
                        node_create_str_ptr(NodeTypeRand, escaped_token),
                        result_list);
            } else if (!strstr(TOKEN_DELIMITERS, token) && !str_equals(token, PRE_POST_STRING_CHAR)) {
                ParserNode *node = rox_map_contains_key(operators, token)
                                   ? node_create_str_copy(NodeTypeRator, token)
                                   : _tokenized_expression_node_from_token(NodeTypeRand, token);
                _tokenized_expression_push_node(expr, node, result_list);
            }
        }
    }

    tokenizer_free(tokenizer);
    tokenized_expression_free(expr);

    return result_list;
}

#undef ROX_TOKEN_BUFFER_SIZE

//
// Parser
//

typedef struct ParserDisposalHandler {
    void *target;
    parser_disposal_handler handler;
} ParserDisposalHandler;

struct Parser {
    RoxList *disposal_handlers;
    RoxMap *operators_map;
};

typedef struct ParserOperator {
    void *target;
    parser_operation operation;
} ParserOperator;

int _parser_compare_stack_items(StackItem *item, StackItem *item2) {
    assert(item);
    assert(item2);

    // we return -1 in case when item types don't match,
    // 1 in case when values are of the same type but not equal,
    // and 0 in case when they are equal.

    int ret_value = -1;
    if (rox_stack_is_null(item)) {
        ret_value = rox_stack_is_null(item2) ? 0 : 1;
    } else if (rox_stack_is_undefined(item)) {
        ret_value = rox_stack_is_undefined(item2) ? 0 : 1;
    } else if (rox_stack_is_numeric(item)) {
        ret_value = rox_stack_is_numeric(item2)
                    ? fabs(rox_stack_get_double(item) - rox_stack_get_double(item2)) < FLT_EPSILON
                      ? 0 : 1
                    : -1;
    } else if (rox_stack_is_boolean(item)) {
        ret_value = rox_stack_is_boolean(item2)
                    ? rox_stack_get_boolean(item) == rox_stack_get_boolean(item2)
                      ? 0 : 1
                    : -1;
    } else if (rox_stack_is_string(item)) {
        ret_value = rox_stack_is_string(item2)
                    ? strcmp(rox_stack_get_string(item), rox_stack_get_string(item2))
                    : -1;
    }
    return ret_value;
}

ROX_INTERNAL void _parser_operator_is_undefined(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    rox_stack_push_boolean(stack, item && rox_stack_is_undefined(item));
}

ROX_INTERNAL void _parser_operator_now(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    double time = current_time_millis();
    rox_stack_push_double(stack, time);
}

ROX_INTERNAL bool _rox_stack_pop_boolean(CoreStack *stack) {
    StackItem *item = rox_stack_pop(stack);
    assert(item);
    if (rox_stack_is_boolean(item)) {
        return rox_stack_get_boolean(item);
    }
    return false;
}

ROX_INTERNAL void _parser_operator_and(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 && b2);
}

ROX_INTERNAL void _parser_operator_or(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 || b2);
}

ROX_INTERNAL void _parser_operator_ne(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    bool equal = _parser_compare_stack_items(item1, item2) == 0;
    rox_stack_push_boolean(stack, !equal);
}

ROX_INTERNAL void _parser_operator_eq(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    bool equal = _parser_compare_stack_items(item1, item2) == 0;
    rox_stack_push_boolean(stack, equal);
}

ROX_INTERNAL void _parser_operator_not(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    rox_stack_push_boolean(stack, !_rox_stack_pop_boolean(stack));
}

ROX_INTERNAL void _parser_operator_if_then(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    bool b = _rox_stack_pop_boolean(stack);
    StackItem *trueExpression = rox_stack_pop(stack);
    StackItem *falseExpression = rox_stack_pop(stack);
    rox_stack_push_item_copy(stack, b ? trueExpression : falseExpression);
}

ROX_INTERNAL void _parser_operator_in_array(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *op1 = rox_stack_pop(stack);
    StackItem *op2 = rox_stack_pop(stack);
    if (!rox_stack_is_list(op2)) {
        rox_stack_push_boolean(stack, false);
        return;
    }
    RoxList *list = rox_stack_get_list(op2);
    RoxDynamicValue *v1 = rox_stack_get_value(op1);
    ROX_LIST_FOREACH(item, list, {
        RoxDynamicValue *v2 = (RoxDynamicValue *) item;
        if (rox_dynamic_value_equals(v1, v2)) {
            rox_stack_push_boolean(stack, true);
            ROX_LIST_FOREACH_RETURN;
        }
    })
    rox_stack_push_boolean(stack, false);
}

ROX_INTERNAL void _parser_operator_md5(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    if (!rox_stack_is_string(item)) {
        rox_stack_push_undefined(stack);
        return;
    }
    char *s = rox_stack_get_string(item);
    rox_stack_push_string_ptr(stack, mem_md5_str(s));
}

ROX_INTERNAL void _parser_operator_concat(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *i1 = rox_stack_pop(stack);
    StackItem *i2 = rox_stack_pop(stack);
    if (!rox_stack_is_string(i1) || !rox_stack_is_string(i2)) {
        rox_stack_push_undefined(stack);
        return;
    }
    char *s1 = rox_stack_get_string(i1);
    char *s2 = rox_stack_get_string(i2);
    rox_stack_push_string_ptr(stack, mem_str_concat(s1, s2));
}

ROX_INTERNAL void _parser_operator_b64d(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    if (!rox_stack_is_string(item)) {
        rox_stack_push_undefined(stack);
        return;
    }
    char *s = rox_stack_get_string(item);
    rox_stack_push_string_ptr(stack, mem_base64_decode_str(s));
}

typedef bool (*parser_comparison_op)(double d1, double d2);

ROX_INTERNAL void
_parser_operator_cmp_dbl(Parser *parser, CoreStack *stack, RoxContext *context, parser_comparison_op cmp) {
    assert(parser);
    assert(stack);
    StackItem *op1 = rox_stack_pop(stack);
    StackItem *op2 = rox_stack_pop(stack);
    if (!rox_stack_is_numeric(op1) ||
        !rox_stack_is_numeric(op2)) {
        rox_stack_push_boolean(stack, false);
        return;
    }
    double d1 = rox_stack_get_double(op1);
    double d2 = rox_stack_get_double(op2);
    bool result = cmp(d1, d2);
    rox_stack_push_boolean(stack, result);
}

bool _parser_cmp_dbl_lt(double d1, double d2) {
    return d2 - d1 > DBL_EPSILON;
}

bool _parser_cmp_dbl_lte(double d1, double d2) {
    return _parser_cmp_dbl_lt(d1, d2) || (fabs(d1 - d2) < DBL_EPSILON);
}

bool _parser_cmp_dbl_gt(double d1, double d2) {
    return d1 - d2 > DBL_EPSILON;
}

bool _parser_cmp_dbl_gte(double d1, double d2) {
    return _parser_cmp_dbl_gt(d1, d2) || (fabs(d1 - d2) < DBL_EPSILON);
}

ROX_INTERNAL void _parser_operator_lt(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_cmp_dbl(parser, stack, context, &_parser_cmp_dbl_lt);
}

ROX_INTERNAL void _parser_operator_lte(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_cmp_dbl(parser, stack, context, &_parser_cmp_dbl_lte);
}

ROX_INTERNAL void _parser_operator_gt(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_cmp_dbl(parser, stack, context, &_parser_cmp_dbl_gt);
}

ROX_INTERNAL void _parser_operator_gte(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_cmp_dbl(parser, stack, context, &_parser_cmp_dbl_gte);
}

typedef int (*_parser_cmp_semver)(semver_t x, semver_t y);

ROX_INTERNAL int _parser_operator_semver_cmp(
        Parser *parser, CoreStack *stack, RoxContext *context, _parser_cmp_semver cmp) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    if (!rox_stack_is_string(item1) || !rox_stack_is_string(item2)) {
        rox_stack_push_boolean(stack, false);
        return -1;
    }
    char *s1 = rox_stack_get_string(item1);
    char *s2 = rox_stack_get_string(item2);
    semver_t v1, v2;
    memset(&v1, 0, sizeof(semver_t));
    memset(&v2, 0, sizeof(semver_t));
    v1.patch = -1;
    v2.patch = -1;
    if (semver_parse(s1, &v1) != 0 || semver_parse(s2, &v2) != 0) {
        rox_stack_push_boolean(stack, false);
        return -1;
    }
    bool result = cmp(v1, v2);
    rox_stack_push_boolean(stack, result);
    return 0;
}

ROX_INTERNAL void _parser_operator_semver_ne(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_neq);
}


ROX_INTERNAL void _parser_operator_semver_eq(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_eq);
}

ROX_INTERNAL void _parser_operator_semver_lt(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_lt);
}

ROX_INTERNAL void _parser_operator_semver_lte(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_lte);
}

ROX_INTERNAL void _parser_operator_semver_gt(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_gt);
}

ROX_INTERNAL void _parser_operator_semver_gte(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    _parser_operator_semver_cmp(parser, stack, context, &semver_gte);
}

ROX_INTERNAL void _parser_operator_match(void *target, Parser *parser, CoreStack *stack, RoxContext *context) {
    assert(parser);
    assert(stack);
    StackItem *op1 = rox_stack_pop(stack);
    StackItem *op2 = rox_stack_pop(stack);
    StackItem *op3 = rox_stack_pop(stack);

    if (!rox_stack_is_string(op1)
        || !rox_stack_is_string(op2)
        || !rox_stack_is_string(op3)) {
        rox_stack_push_boolean(stack, false);
        return;
    }

    const char *str = rox_stack_get_string(op1);
    const char *pattern = rox_stack_get_string(op2);
    const char *flags = rox_stack_get_string(op3);

    unsigned int options = 0;
    for (int i = 0; flags[i] != '\0'; ++i) {
        char flag = flags[i];
        if (flag == 'i') {
            options |= PCRE2_CASELESS;
        }
        if (flag == 'x') {
            options |= PCRE2_EXTENDED;
        }
        if (flag == 's') {
            options |= PCRE2_DOTALL;
        }
        if (flag == 'm') {
            options |= PCRE2_MULTILINE;
        }
        if (flag == 'n') {
            options |= PCRE2_NO_AUTO_CAPTURE;
        }
    }

    bool match = str_matches(str, pattern, options);
    rox_stack_push_boolean(stack, match);
}

ROX_INTERNAL void _parser_set_basic_operators(Parser *parser) {
    assert(parser);

    // basic functions
    parser_add_operator(parser, "isUndefined", NULL, &_parser_operator_is_undefined);
    parser_add_operator(parser, "now", NULL, &_parser_operator_now);
    parser_add_operator(parser, "and", NULL, &_parser_operator_and);
    parser_add_operator(parser, "or", NULL, &_parser_operator_or);
    parser_add_operator(parser, "ne", NULL, &_parser_operator_ne);
    parser_add_operator(parser, "eq", NULL, &_parser_operator_eq);
    parser_add_operator(parser, "not", NULL, &_parser_operator_not);
    parser_add_operator(parser, "ifThen", NULL, &_parser_operator_if_then);
    parser_add_operator(parser, "inArray", NULL, &_parser_operator_in_array);
    parser_add_operator(parser, "md5", NULL, &_parser_operator_md5);
    parser_add_operator(parser, "concat", NULL, &_parser_operator_concat);
    parser_add_operator(parser, "b64d", NULL, &_parser_operator_b64d);

    // value compare functions
    parser_add_operator(parser, "lt", NULL, &_parser_operator_lt);
    parser_add_operator(parser, "lte", NULL, &_parser_operator_lte);
    parser_add_operator(parser, "gt", NULL, &_parser_operator_gt);
    parser_add_operator(parser, "gte", NULL, &_parser_operator_gte);
    parser_add_operator(parser, "semverNe", NULL, &_parser_operator_semver_ne);
    parser_add_operator(parser, "semverEq", NULL, &_parser_operator_semver_eq);
    parser_add_operator(parser, "semverLt", NULL, &_parser_operator_semver_lt);
    parser_add_operator(parser, "semverLte", NULL, &_parser_operator_semver_lte);
    parser_add_operator(parser, "semverGt", NULL, &_parser_operator_semver_gt);
    parser_add_operator(parser, "semverGte", NULL, &_parser_operator_semver_gte);

    // regular expression functions
    parser_add_operator(parser, "match", NULL, &_parser_operator_match);
}

ROX_INTERNAL Parser *parser_create() {
    Parser *parser = calloc(1, sizeof(Parser));
    parser->disposal_handlers = rox_list_create();
    parser->operators_map = rox_map_create();
    _parser_set_basic_operators(parser);
    return parser;
}

ROX_INTERNAL void parser_add_disposal_handler(
        Parser *parser,
        void *target,
        parser_disposal_handler handler) {

    assert(parser);
    assert(handler);

    ParserDisposalHandler *h = calloc(1, sizeof(ParserDisposalHandler));
    h->target = target;
    h->handler = handler;
    rox_list_add(parser->disposal_handlers, h);
}

ROX_INTERNAL void parser_free(Parser *parser) {
    assert(parser);
    ROX_LIST_FOREACH(item, parser->disposal_handlers, {
        ParserDisposalHandler *handler = (ParserDisposalHandler *) item;
        handler->handler(handler->target, parser);
    })
    rox_map_free_with_keys_and_values(parser->operators_map);
    rox_list_free_cb(parser->disposal_handlers, &free);
    free(parser);
}

ROX_INTERNAL void parser_add_operator(Parser *parser, const char *name, void *target, parser_operation op) {
    assert(parser);
    assert(name);
    assert(op);
    assert(!rox_map_contains_key(parser->operators_map, (void *) name));
    ParserOperator *operator = calloc(1, sizeof(ParserOperator));
    operator->target = target;
    operator->operation = op;
    rox_map_add(parser->operators_map, (void *) mem_copy_str(name), operator);
}

ROX_INTERNAL EvaluationResult *parser_evaluate_expression(Parser *parser, const char *expression, RoxContext *context) {
    assert(parser);
    assert(expression);

    EvaluationResult *result = NULL;
    StackItem *item = NULL;
    CoreStack *stack = rox_stack_create();
    RoxList *tokens = tokenized_expression_get_tokens(expression, parser->operators_map);
    rox_list_reverse(tokens);

    RoxListIter *i = rox_list_iter_create();
    rox_list_iter_init(i, tokens);
    ParserNode *node;
    while (rox_list_iter_next(i, (void **) &node)) {
        if (node->type == NodeTypeRand) {
            rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_copy(node->value));
        } else if (node->type == NodeTypeRator) {
            assert(rox_dynamic_value_is_string(node->value));
            ParserOperator *op;
            if (rox_map_get(parser->operators_map, rox_dynamic_value_get_string(node->value), (void **) &op)) {
                op->operation(op->target, parser, stack, context);
            }
        } else {
            result = _create_result_from_stack_item(NULL);
        }
    }

    rox_list_iter_free(i);

    if (!result) {
        item = rox_stack_pop(stack);
        result = _create_result_from_stack_item(item);
    }

    rox_list_free_cb(tokens, (void (*)(void *)) &node_free); // here all the inner lists and maps should be freed
    rox_stack_free(stack);

    return result;
}