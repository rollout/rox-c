#pragma once

#include "rox/context.h"
#include "core/eval.h"
#include "stack.h"

typedef struct Parser Parser;

typedef void (*parser_operation)(void *target, Parser *parser, CoreStack *stack, EvaluationContext *eval_context);

typedef void (*parser_disposal_handler)(void *target, Parser *parser);

typedef enum ParserTokenType {
    TokenTypeString,
    TokenTypeBool,
    TokenTypeNumber,
    TokenTypeUndefined,
    TokenTypeNotAType
} ParserTokenType;

ROX_INTERNAL ParserTokenType get_token_type_from_token(const char *token);

typedef enum NodeType {
    NodeTypeRand,
    NodeTypeRator,
    NodeTypeUnknown
} NodeType;

typedef struct ParserNode ParserNode;

ROX_INTERNAL NodeType node_get_type(ParserNode *node);

ROX_INTERNAL RoxDynamicValue *node_get_value(ParserNode *node);

ROX_INTERNAL void node_free(ParserNode *node);

/**
 * NOTE: THE RETURNED LIST MUST BE FREED BY CALLING list_destroy_cb(tokens, node_free) AFTER USE.
 *
 * @param expression Expression to parse.
 * @param operators Set of supported operator names (set of char* )
 * @return List of ParserNode*
 */
ROX_INTERNAL RoxList *tokenized_expression_get_tokens(const char *expression, RoxMap *operators);

ROX_INTERNAL Parser *parser_create();

/**
 * The passed handler will be invoked upon calling <code>parser_free()</code>.
 *
 * @param parser NOT <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler NOT <code>NULL</code>.
 */
ROX_INTERNAL void parser_add_disposal_handler(Parser *parser, void *target, parser_disposal_handler handler);

ROX_INTERNAL void parser_free(Parser *parser);

/**
 * @param parser Parser reference. NOT <code>NULL</code>.
 * @param name Operator name. NOT <code>NULL</code>. Value is copied internally. The caller is responsible for freeing the memory.
 * @param target Optional function target. May be <code>NULL</code>.
 * @param op Pointer to operation function. NOT <code>NULL</code>.
 */
ROX_INTERNAL void parser_add_operator(Parser *parser, const char *name, void *target, parser_operation op);

/**
 * THE RETURNED POINTER MUST BE FREED AFTER USE BY CALLING result_free(result).
 * @param parser Parser reference. NOT NULL.
 * @param expression Expression. NOT NULL.
 * @param context Can be NULL.
 * @return Can be NULL.
 */
ROX_INTERNAL EvaluationResult *parser_evaluate_expression(
        Parser *parser,
        const char *expression,
        EvaluationContext *eval_context);

//
// Get value from evaluation result. If actual value type is different
// cast is performed, so it's safe to call any of these func on any EvaluationResult.
//

ROX_INTERNAL EvaluationResult *result_create(RoxContext *context);

ROX_INTERNAL RoxContext *result_get_context(EvaluationResult *result);

ROX_INTERNAL bool result_is_undefined(EvaluationResult *result);

ROX_INTERNAL int *result_get_int(EvaluationResult *result);

ROX_INTERNAL double *result_get_double(EvaluationResult *result);

ROX_INTERNAL bool *result_get_boolean(EvaluationResult *result);

ROX_INTERNAL char *result_get_string(EvaluationResult *result);

/**
 * DON'T FORGET TO CALL THIS.
 * @param result A Non-NULL pointer to EvaluationResult returned by parser_evaluate_expression().
 */
ROX_INTERNAL void result_free(EvaluationResult *result);
