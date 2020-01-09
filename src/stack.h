#pragma once

#include "visibility.h"

typedef struct Stack Stack;

Stack *ROX_INTERNAL stack_create();

void ROX_INTERNAL stack_free(Stack *stack);

int ROX_INTERNAL stack_is_full(Stack *stack);

int ROX_INTERNAL stack_is_empty(Stack *stack);

void ROX_INTERNAL stack_push(Stack **stack, void *item);

void *ROX_INTERNAL stack_pop(Stack **stack);

void *ROX_INTERNAL stack_peek(Stack *stack);