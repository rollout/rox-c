#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pcre2.h>
#include <collectc/list.h>
#include <collectc/hashtable.h>

#include "roxapi.h"
#include "util.h"
#include "core/entities.h"
#include "roxx/parser.h"
#include "roxx/stack.h"

//
// Symbols
//

const char *ROX_INTERNAL ROXX_UNDEFINED = "undefined";
const char *ROX_INTERNAL ROXX_TRUE = "true";
const char *ROX_INTERNAL ROXX_FALSE = "false";
const char *ROX_INTERNAL ROXX_EMPTY_STRING = "\"\"";

//
// TokenTypes
//

typedef enum ROX_INTERNAL {
    TokenTypeString,
    TokenTypeBool,
    TokenTypeNumber,
    TokenTypeUndefined,
    TokenTypeNotAType
} TokenType;

TokenType ROX_INTERNAL get_token_type_from_token(const char *token) {
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

struct ROX_INTERNAL EvaluationResult {
    int *int_value;
    double *double_value;
    char *str_value;
    bool *bool_value;
};

EvaluationResult *ROX_INTERNAL _create_result_from_stack_item(StackItem *item) {
    EvaluationResult *result = calloc(1, sizeof(EvaluationResult));

    if (!item || rox_stack_is_null(item)) {
        return result;
    }

    if (rox_stack_is_undefined(item)) {
        result->str_value = mem_copy_str(ROXX_UNDEFINED);
        return result;
    }

    if (rox_stack_is_boolean(item)) {
        result->bool_value = malloc(sizeof(bool));
        *result->bool_value = rox_stack_get_boolean(item);
        result->str_value = mem_copy_str(*result->bool_value
                                         ? FLAG_TRUE_VALUE
                                         : FLAG_FALSE_VALUE);
        return result;
    }

    if (rox_stack_is_int(item)) {
        int value = rox_stack_get_int(item);
        result->int_value = mem_copy_int(value);
        result->double_value = mem_copy_double(value);
        result->str_value = mem_int_to_str(value);
        return result;
    }

    if (rox_stack_is_double(item)) {
        double value = rox_stack_get_double(item);
        result->int_value = mem_copy_int((int) value);
        result->double_value = mem_copy_double(value);
        result->str_value = mem_double_to_str(value);
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
    NodeTypeRand,
    NodeTypeRator,
    NodeTypeUnknown
} NodeType;

typedef struct ROX_INTERNAL ParserNode {
    NodeType type;
    char *str_value;
    int *int_value;
    double *double_value;
    bool *bool_value;
    List *list_value;
    HashTable *map_value;
    bool is_undefined;
    bool is_null;
} ParserNode;

ParserNode *ROX_INTERNAL _node_create_empty(NodeType type) {
    ParserNode *node = (ParserNode *) calloc(1, sizeof(ParserNode));
    node->type = type;
    return node;
}

ParserNode *ROX_INTERNAL node_create_str(NodeType type, const char *str) {
    assert(str);
    ParserNode *node = _node_create_empty(type);
    node->str_value = mem_copy_str(str);
    return node;
}

ParserNode *ROX_INTERNAL node_create_int(NodeType type, int value) {
    ParserNode *node = _node_create_empty(type);
    node->int_value = mem_copy_int(value);
    node->str_value = mem_int_to_str(value);
    return node;
}

ParserNode *ROX_INTERNAL node_create_double(NodeType type, double value) {
    ParserNode *node = _node_create_empty(type);
    node->double_value = mem_copy_double(value);
    node->str_value = mem_double_to_str(value);
    return node;
}

ParserNode *ROX_INTERNAL node_create_bool(NodeType type, bool value) {
    ParserNode *node = _node_create_empty(type);
    node->bool_value = mem_copy_bool(value);
    node->str_value = mem_bool_to_str(value, ROXX_TRUE, ROXX_FALSE);
    return node;
}

ParserNode *ROX_INTERNAL node_create_list(NodeType type, List *value) {
    assert(value);
    ParserNode *node = _node_create_empty(type);
    node->list_value = value; // NOTE: collections aren't copied
    return node;
}

ParserNode *ROX_INTERNAL node_create_map(NodeType type, HashTable *value) {
    assert(value);
    ParserNode *node = _node_create_empty(type);
    node->map_value = value; // NOTE: collections aren't copied
    return node;
}

ParserNode *ROX_INTERNAL node_create_null(NodeType type) {
    ParserNode *node = _node_create_empty(type);
    node->is_null = true;
    return node;
}

ParserNode *ROX_INTERNAL node_create_undefined(NodeType type) {
    ParserNode *node = _node_create_empty(type);
    node->is_undefined = true;
    return node;
}

void ROX_INTERNAL node_free(ParserNode *node) {
    assert(node);
    if (node->str_value) {
        free(node->str_value);
    }
    if (node->int_value) {
        free(node->int_value);
    }
    if (node->double_value) {
        free(node->double_value);
    }
    if (node->bool_value) {
        free(node->bool_value);
    }
    if (node->list_value) {
        list_destroy(node->list_value);
    }
    if (node->map_value) {
        hashtable_destroy(node->map_value);
    }
    free(node);
}

//
// StringTokenizer.
// C port of Java's StringTokenizer.
//

typedef struct ROX_INTERNAL StringTokenizer {
    int current_position;
    int new_position;
    int max_position;
    char *str;
    char *delimiters;
    bool ret_delims;
    bool delims_changed;
    int max_delim_code_point;
} StringTokenizer;

void ROX_INTERNAL _tokenizer_set_max_delim_code_point(StringTokenizer *tokenizer) {
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

StringTokenizer *ROX_INTERNAL tokenizer_create(const char *str, const char *delim, bool return_delims) {
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

void ROX_INTERNAL tokenizer_free(StringTokenizer *tokenizer) {
    assert(tokenizer);
    free(tokenizer->delimiters);
    free(tokenizer->str);
    free(tokenizer);
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
void ROX_INTERNAL tokenizer_next_token(StringTokenizer *tokenizer, char *buffer) {
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
    str_substring_b(tokenizer->str, start, (tokenizer->current_position - start), buffer);
}

void ROX_INTERNAL tokenizer_next_token_with_delim(StringTokenizer *tokenizer, const char *delim, char *buffer) {
    assert(tokenizer);
    assert(delim);
    assert(buffer);

    free(tokenizer->delimiters);
    tokenizer->delimiters = mem_copy_str(delim);

    /* delimiter string specified, so set the appropriate flag. */
    tokenizer->delims_changed = true;

    _tokenizer_set_max_delim_code_point(tokenizer);
    tokenizer_next_token(tokenizer, buffer);
}

//
// TokenizedExpression
//

const char *DICT_START_DELIMITER = "{";
const char *DICT_END_DELIMITER = "}";
const char *ARRAY_START_DELIMITER = "[";
const char *ARRAY_END_DELIMITER = "]";
const char *TOKEN_DELIMITERS = "{}[]():, \t\r\n\"";
const char *PRE_POST_STRING_CHAR = "";
const char *STRING_DELIMITER = "\"";

const char *ESCAPED_QUOTE = "\\\"";
const char *ESCAPED_QUOTE_PLACEHOLDER = "\\RO_Q";

typedef struct ROX_INTERNAL TokenizedExpression {
    List *array_accumulator;
    HashTable *dict_accumulator;
    char *dict_key;
} TokenizedExpression;

void ROX_INTERNAL tokenized_expression_free(TokenizedExpression *expr) {
    assert(expr);
    if (expr->array_accumulator) {
        list_destroy(expr->array_accumulator); // FIXME: add warning: unclosed array literal
    }
    if (expr->dict_accumulator) {
        hashtable_destroy(expr->dict_accumulator);  // FIXME: add warning: unclosed dictionary literal
    }
    free(expr);
}

void ROX_INTERNAL _tokenized_expression_push_node(TokenizedExpression *expr, ParserNode *node, List *node_list) {
    assert(expr);
    assert(node);
    assert(node_list);
    if (expr->dict_accumulator && !expr->dict_key) {
        expr->dict_key = node->str_value;
    } else if (expr->dict_accumulator && expr->dict_key) {
        if (!hashtable_contains_key(expr->dict_accumulator, expr->dict_key)) {
            hashtable_add(expr->dict_accumulator, expr->dict_key, node);
        }
        expr->dict_key = NULL;
    } else if (expr->array_accumulator) {
        list_add(expr->array_accumulator, node);
    } else {
        list_add(node_list, node);
    }
}

ParserNode *ROX_INTERNAL _tokenized_expression_node_from_token(NodeType nodeType, const char *token) {
    assert(token);
    TokenType token_type = get_token_type_from_token(token);
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
        char buffer[512];
        str_substring_b(token, 1, token_length - 2, buffer);
        return node_create_str(nodeType, buffer); // buffer contents will be copied
    }
    if (token_type == TokenTypeNumber) {
        double *value = mem_str_to_double(token);
        assert(value); // TODO: log if null
        double node_value = *value;
        free(value);
        return node_create_double(nodeType, node_value);
    }
    return node_create_null(NodeTypeUnknown);
}

/**
 * NOTE: THE RETURNED LIST MUST BE FREED BY CALLING list_destroy(list) AFTER USE.
 *
 * @param expression Expression to parse.
 * @param operators Set of supported operator names (set of char* )
 * @return List of ParserNode*
 */
List *ROX_INTERNAL tokenized_expression_get_tokens(const char *expression, HashTable *operators) {
    assert(expression);
    assert(operators);

    TokenizedExpression *expr = calloc(1, sizeof(TokenizedExpression));

    List *result_list;
    list_new(&result_list);

    const char *delimiters_to_use = TOKEN_DELIMITERS;
    char *normalized_expression = mem_str_replace(expression, ESCAPED_QUOTE, ESCAPED_QUOTE_PLACEHOLDER);
    StringTokenizer *tokenizer = tokenizer_create(normalized_expression, delimiters_to_use, true);
    free(normalized_expression);

    char *prev_token = NULL;
    char token[1024];
    while (tokenizer_has_more_tokens(tokenizer)) {
        prev_token = token;
        tokenizer_next_token_with_delim(tokenizer, delimiters_to_use, token);
        bool in_string = str_equals(delimiters_to_use, STRING_DELIMITER);

        if (!in_string && str_equals(token, DICT_START_DELIMITER)) {
            if (expr->dict_accumulator) {
                // TODO: log illegal case warning? (new dict has started before the existing is closed)
                hashtable_destroy(expr->dict_accumulator); // FIXME: what about dict-in-dict case?
            }
            hashtable_new(&expr->dict_accumulator);
        } else if (!in_string && str_equals(token, DICT_END_DELIMITER)) {
            HashTable *dict_result = expr->dict_accumulator;
            expr->dict_accumulator = NULL;
            _tokenized_expression_push_node(
                    expr,
                    node_create_map(NodeTypeRand, dict_result),  // NOTE: dict_result must be freed in node_free
                    result_list);
        } else if (!in_string && str_equals(token, ARRAY_START_DELIMITER)) {
            if (expr->array_accumulator) {
                // TODO: log illegal case warning? (new array has started before the existing is closed)
                list_destroy(expr->array_accumulator);  // FIXME: what about array-in-array case?
            }
            list_new(&expr->array_accumulator);
        } else if (!in_string && str_equals(token, ARRAY_END_DELIMITER)) {
            List *array_result = expr->array_accumulator;
            expr->array_accumulator = NULL;
            _tokenized_expression_push_node(
                    expr,
                    node_create_list(NodeTypeRand, array_result), // NOTE: array_result must be freed in node_free
                    result_list);
        } else if (str_equals(token, STRING_DELIMITER)) {
            if (prev_token && str_equals(prev_token, STRING_DELIMITER)) {
                _tokenized_expression_push_node(
                        expr,
                        node_create_str(NodeTypeRand, ROXX_EMPTY_STRING),
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
                        _tokenized_expression_node_from_token(NodeTypeRand, escaped_token),
                        result_list);
                free(escaped_token);
            } else if (!strstr(TOKEN_DELIMITERS, token) && !str_equals(token, PRE_POST_STRING_CHAR)) {
                ParserNode *node = hashtable_contains_key(operators, token)
                                   ? node_create_str(NodeTypeRator, token)
                                   : _tokenized_expression_node_from_token(NodeTypeRand, token);
                _tokenized_expression_push_node(expr, node, result_list);
            }
        }
    }

    tokenizer_free(tokenizer);
    tokenized_expression_free(expr);

    return result_list;
}

//
// Parser
//

struct ROX_INTERNAL Parser {
    HashTable *operators_map;
};

void ROX_INTERNAL _parser_operator_is_undefined(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    rox_stack_push_boolean(stack, item && rox_stack_is_undefined(item));
}

void ROX_INTERNAL _parser_operator_now(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    rox_stack_push_double(stack, current_time_millis());
}

bool ROX_INTERNAL _rox_stack_pop_boolean(CoreStack *stack) {
    StackItem *item = rox_stack_pop(stack);
    assert(item);
    if (rox_stack_is_boolean(item)) {
        return rox_stack_get_boolean(item);
    }
    return false;
}

void ROX_INTERNAL _parser_operator_and(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 && b2);
}

void ROX_INTERNAL _parser_operator_or(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 || b2);
}

void ROX_INTERNAL _parser_operator_ne(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 != b2);
}

void ROX_INTERNAL _parser_operator_eq(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    bool b1 = _rox_stack_pop_boolean(stack);
    bool b2 = _rox_stack_pop_boolean(stack);
    rox_stack_push_boolean(stack, b1 == b2);
}

void ROX_INTERNAL _parser_operator_not(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    rox_stack_push_boolean(stack, !_rox_stack_pop_boolean(stack));
}

void ROX_INTERNAL _parser_operator_if_then(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    bool b = _rox_stack_pop_boolean(stack);
    StackItem *trueExpression = rox_stack_pop(stack);
    StackItem *falseExpression = rox_stack_pop(stack);
    rox_stack_push_item_copy(stack, b ? trueExpression : falseExpression);
}

int _parser_compare_stack_item_with_node(const StackItem *item, const ParserNode *node) {
    assert(item);
    assert(node);
    // TODO: implement!
    return -1;
}

void ROX_INTERNAL _parser_operator_in_array(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *op1 = rox_stack_pop(stack);
    StackItem *op2 = rox_stack_pop(stack);
    if (!rox_stack_is_list(op1)) {
        rox_stack_push_boolean(stack, false);
        return;
    }
    List *list = rox_stack_get_list(op1);
    rox_stack_push_boolean(stack, list_contains_value(
            list, op2, (int (*)(const void *, const void *))
                    &_parser_compare_stack_item_with_node));
}

void ROX_INTERNAL _parser_operator_md5(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    if (!rox_stack_is_string(item)) {
        rox_stack_push_undefined(stack);
        return;
    }
    char *s = rox_stack_get_string(item);
    rox_stack_push_string_ptr(stack, mem_md5(s));
}

void ROX_INTERNAL _parser_operator_concat(Parser *parser, CoreStack *stack, Context *context) {
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

void ROX_INTERNAL _parser_operator_b64d(Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item = rox_stack_pop(stack);
    if (!rox_stack_is_string(item)) {
        rox_stack_push_undefined(stack);
        return;
    }
    char *s = rox_stack_get_string(item);
    rox_stack_push_string_ptr(stack, mem_base64_decode(s));
}

void ROX_INTERNAL _parser_set_basic_operators(Parser *parser) {
    assert(parser);
    parser_add_operator(parser, "isUndefined", &_parser_operator_is_undefined);
    parser_add_operator(parser, "now", &_parser_operator_now);
    parser_add_operator(parser, "and", &_parser_operator_and);
    parser_add_operator(parser, "or", &_parser_operator_or);
    parser_add_operator(parser, "ne", &_parser_operator_ne);
    parser_add_operator(parser, "eq", &_parser_operator_eq);
    parser_add_operator(parser, "not", &_parser_operator_not);
    parser_add_operator(parser, "ifThen", &_parser_operator_if_then);
    parser_add_operator(parser, "inArray", &_parser_operator_in_array);
    parser_add_operator(parser, "md5", &_parser_operator_md5);
    parser_add_operator(parser, "concat", &_parser_operator_concat);
    parser_add_operator(parser, "b64d", &_parser_operator_b64d);
    // TODO: value compare functions
    // TODO: regular expression functions
}

Parser *ROX_INTERNAL parser_create() {
    Parser *parser = calloc(1, sizeof(Parser));
    hashtable_new(&parser->operators_map);
    _parser_set_basic_operators(parser);
    return parser;
}

void ROX_INTERNAL parser_free(Parser *parser) {
    assert(parser);
    hashtable_destroy(parser->operators_map);
    free(parser);
}

void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, parser_operation op) {
    assert(parser);
    assert(name);
    assert(op);
    assert(!hashtable_contains_key(parser->operators_map, (void *) name));
    hashtable_add(parser->operators_map, (void *) name, op);
}

void ROX_INTERNAL _stack_push_node_data(CoreStack *stack, ParserNode *node) {
    assert(stack);
    assert(node);
    if (node->int_value) {
        rox_stack_push_int(stack, *node->int_value);
    } else if (node->double_value) {
        rox_stack_push_double(stack, *node->double_value);
    } else if (node->bool_value) {
        rox_stack_push_boolean(stack, *node->bool_value);
    } else if (node->list_value) {
        rox_stack_push_list(stack, node->list_value);
    } else if (node->map_value) {
        rox_stack_push_map(stack, node->map_value);
    } else if (node->is_undefined) {
        rox_stack_push_undefined(stack);
    } else if (node->is_null) {
        rox_stack_push_null(stack);
    }
    // node->str_value may be set for other primitive types as well, and contain str
    // representation of them, so checking str_value last.
    if (node->str_value) {
        rox_stack_push_string_copy(stack, node->str_value);
    }
}

EvaluationResult *ROX_INTERNAL parser_evaluate_expression(Parser *parser, const char *expression, Context *context) {
    assert(parser);
    assert(expression);

    EvaluationResult *result = NULL;
    StackItem *item = NULL;
    CoreStack *stack = rox_stack_create();
    List *tokens = tokenized_expression_get_tokens(expression, parser->operators_map);
    list_reverse(tokens);

    LIST_FOREACH(token, tokens, {
        ParserNode *node = token;
        if (node->type == NodeTypeRand) {
            _stack_push_node_data(stack, node);
        } else if (node->type == NodeTypeRator) {
            assert(node->str_value);
            parser_operation op;
            if (hashtable_get(parser->operators_map, node->str_value, (void **) &op) == CC_OK) {
                op(parser, stack, context);
            }
        } else {
            result = _create_result_from_stack_item(NULL);
        }
    })

    if (!result) {
        item = rox_stack_pop(stack);
        result = _create_result_from_stack_item(item);
    }

    list_destroy_cb(tokens, (void (*)(void *)) &node_free); // here all the inner lists and maps should be destroyed
    rox_stack_free(stack);

    return result;
}