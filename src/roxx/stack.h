#pragma once

#include <stdbool.h>
#include "visibility.h"

typedef struct CoreStack CoreStack;
typedef struct StackItem StackItem;

//
// Stack manipulation.
//
// In ROX stack may contain various data types such as ints, floats, strings and so on.
// Here we define functions to create and use stack, and destroy it once it is not under use.
//

CoreStack *ROX_INTERNAL stack_create();

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, stack item otherwise.
 */
StackItem *ROX_INTERNAL stack_pop(CoreStack *stack);

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, current stack item otherwise.
 */
StackItem *ROX_INTERNAL stack_peek(CoreStack *stack);

/**
 *
 * @param stack
 * @return
 */
bool ROX_INTERNAL stack_is_empty(CoreStack *stack);

void ROX_INTERNAL stack_push_int(CoreStack *stack, int value);

void ROX_INTERNAL stack_push_float(CoreStack *stack, float value);

void ROX_INTERNAL stack_push_double(CoreStack *stack, double value);

void ROX_INTERNAL stack_push_boolean(CoreStack *stack, bool value);

void ROX_INTERNAL stack_push_string(CoreStack *stack, const char *value);

/**
 * @param stack A NON-NULL pointer to the stack.
 */
void ROX_INTERNAL stack_free(CoreStack *stack);

//
// Stack item contents type check.
//
// To check which type the stack item has one of the following functions should be called.
//

bool ROX_INTERNAL is_int_value(StackItem *item);

bool ROX_INTERNAL is_float_value(StackItem *item);

bool ROX_INTERNAL is_double_value(StackItem *item);

bool ROX_INTERNAL is_boolean_value(StackItem *item);

bool ROX_INTERNAL is_string_value(StackItem *item);

//
// Stack item contents retrieval.
//
// Once stack item type is determined, one of the following get_***_value functions should be
// called to obtain the actual value. NOTE: the returned value is a constant pointer to the
// stack item data, and it shouldn't be freed directly. Instead, stack_free function should be called
// on the entire stack object.
//

int ROX_INTERNAL get_int_value(StackItem *item);

float ROX_INTERNAL get_float_value(StackItem *item);

double ROX_INTERNAL get_double_value(StackItem *item);

bool ROX_INTERNAL get_boolean_value(StackItem *item);

char *ROX_INTERNAL get_string_value(StackItem *item);
