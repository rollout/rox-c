#pragma once

#include "visibility.h"
#include "stack.h"
#include "context.h"

struct Parser;

typedef void ROX_INTERNAL (*parser_operation(struct Parser *parser, Stack *stack));

typedef struct Parser Parser;

void ROX_INTERNAL parser_evaluate_expression(const char *expression, Context *context);

void ROX_INTERNAL parser_add_operator(Parser *parser, const char *name, parser_operation op);
