#pragma once

#include "roxapi.h"
#include "roxx/stack.h"
#include "context.h"

typedef struct ROX_INTERNAL Parser Parser;

typedef struct ROX_INTERNAL EvaluationResult EvaluationResult;

typedef void ROX_INTERNAL (*parser_operation)(Parser *parser, CoreStack *stack, Context *context);

typedef enum ROX_INTERNAL {
    TokenTypeString,
    TokenTypeBool,
    TokenTypeNumber,
    TokenTypeUndefined,
    TokenTypeNotAType
} TokenType;

TokenType ROX_INTERNAL get_token_type_from_token(const char *token);

typedef enum ROX_INTERNAL NodeType {
    NodeTypeRand,
    NodeTypeRator,
    NodeTypeUnknown
} NodeType;

typedef struct ROX_INTERNAL ParserNode {
    NodeType type;
    char *str_value;
    double *double_value;
    List *list_value;
    HashTable *map_value;
    bool is_undefined;
    bool is_null;
    bool is_true;
    bool is_false;
} ParserNode;

void ROX_INTERNAL node_free(ParserNode *node);

/**
 * NOTE: THE RETURNED LIST MUST BE FREED BY CALLING list_destroy_cb(tokens, node_free) AFTER USE.
 *
 * @param expression Expression to parse.
 * @param operators Set of supported operator names (set of char* )
 * @return List of ParserNode*
 */
List *ROX_INTERNAL tokenized_expression_get_tokens(const char *expression, HashTable *operators);

Parser *ROX_INTERNAL parser_create();

void ROX_INTERNAL parser_free(Parser *parser);

/**
 * @param parser Parser reference. NOT NULL.
 * @param name Operator name. NOT NULL.
 * @param op Pointer to operation function. NOT NULL.
 */
void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, parser_operation op);

/**
 * @param parser Parser reference. NOT NULL.
 * @param expression Expression. NOT NULL.
 * @param context Can be NULL.
 * @return Not NULL.
 */
EvaluationResult *ROX_INTERNAL parser_evaluate_expression(Parser *parser, const char *expression, Context *context);

//
// Get value from evaluation result. If actual value type is different
// cast is performed, so it's safe to call any of these func on any EvaluationResult.
//

int *ROX_INTERNAL result_get_int(EvaluationResult *result);

double *ROX_INTERNAL result_get_double(EvaluationResult *result);

bool ROX_INTERNAL result_get_boolean(EvaluationResult *result);

char *ROX_INTERNAL result_get_string(EvaluationResult *result);

/**
 * DON'T FORGET TO CALL THIS.
 * @param result A Non-NULL pointer to EvaluationResult returned by parser_evaluate_expression().
 */
void ROX_INTERNAL result_free(EvaluationResult *result);
