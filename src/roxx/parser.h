#pragma once

#include "visibility.h"
#include "roxx/stack.h"
#include "context.h"

typedef struct Parser Parser;

typedef void ROX_INTERNAL (*parser_operation(struct Parser *parser, CoreStack *stack));

void ROX_INTERNAL parser_evaluate_expression(const char *expression, Context *context);

void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, parser_operation op);
