#pragma once

#include "roxapi.h"
#include "roxx/stack.h"
#include "context.h"

typedef struct ROX_INTERNAL Parser Parser;

typedef struct ROX_INTERNAL EvaluationResult EvaluationResult;

typedef void ROX_INTERNAL (*parser_operation(struct Parser *parser, CoreStack *stack));

void ROX_INTERNAL parser_evaluate_expression(const char *expression, Context *context);

void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, parser_operation op);

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
