#pragma once

#include "rollout.h"
#include "stack.h"
#include "dynamic.h"
#include "core/context.h"

typedef struct ROX_INTERNAL Parser Parser;

typedef struct ROX_INTERNAL EvaluationResult EvaluationResult;

typedef ROX_INTERNAL void (*parser_operation)(void *target, Parser *parser, CoreStack *stack, RoxContext *context);

typedef ROX_INTERNAL void (*parser_disposal_handler)(void *target, Parser *parser);

typedef enum ROX_INTERNAL ParserTokenType {
    TokenTypeString,
    TokenTypeBool,
    TokenTypeNumber,
    TokenTypeUndefined,
    TokenTypeNotAType
} ParserTokenType;

ParserTokenType ROX_INTERNAL get_token_type_from_token(const char *token);

typedef enum ROX_INTERNAL NodeType {
    NodeTypeRand,
    NodeTypeRator,
    NodeTypeUnknown
} NodeType;

typedef struct ROX_INTERNAL ParserNode ParserNode;

NodeType ROX_INTERNAL node_get_type(ParserNode *node);

RoxDynamicValue *ROX_INTERNAL node_get_value(ParserNode *node);

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

/**
 * The passed handler will be invoked upon calling <code>parser_free()</code>.
 *
 * @param parser NOT <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler NOT <code>NULL</code>.
 */
void ROX_INTERNAL parser_add_disposal_handler(Parser *parser, void *target, parser_disposal_handler handler);

void ROX_INTERNAL parser_free(Parser *parser);

/**
 * @param parser Parser reference. NOT <code>NULL</code>.
 * @param name Operator name. NOT <code>NULL</code>. Value is copied internally. The caller is responsible for freeing the memory.
 * @param target Optional function target. May be <code>NULL</code>.
 * @param op Pointer to operation function. NOT <code>NULL</code>.
 */
void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, void *target, parser_operation op);

/**
 * THE RETURNED POINTER MUST BE FREED AFTER USE BY CALLING result_free(result).
 * @param parser Parser reference. NOT NULL.
 * @param expression Expression. NOT NULL.
 * @param context Can be NULL.
 * @return Not NULL.
 */
EvaluationResult *ROX_INTERNAL parser_evaluate_expression(Parser *parser, const char *expression, RoxContext *context);

//
// Get value from evaluation result. If actual value type is different
// cast is performed, so it's safe to call any of these func on any EvaluationResult.
//

int *ROX_INTERNAL result_get_int(EvaluationResult *result);

double *ROX_INTERNAL result_get_double(EvaluationResult *result);

bool *ROX_INTERNAL result_get_boolean(EvaluationResult *result);

char *ROX_INTERNAL result_get_string(EvaluationResult *result);

/**
 * DON'T FORGET TO CALL THIS.
 * @param result A Non-NULL pointer to EvaluationResult returned by parser_evaluate_expression().
 */
void ROX_INTERNAL result_free(EvaluationResult *result);
