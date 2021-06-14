#pragma once

#include <stdbool.h>
#include "rox/server.h"

typedef struct CoreStack CoreStack;
typedef struct StackItem StackItem;

//
// Stack manipulation.
//
// In ROX stack may contain various data types such as ints, floats, strings and so on.
// Here we define functions to create and use stack, and destroy it once it is not under use.
//

ROX_INTERNAL CoreStack *rox_stack_create();

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, stack item otherwise.
 */
ROX_INTERNAL StackItem *rox_stack_pop(CoreStack *stack);

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, current stack item otherwise.
 */
ROX_INTERNAL StackItem *rox_stack_peek(CoreStack *stack);

ROX_INTERNAL bool rox_stack_is_empty(CoreStack *stack);

ROX_INTERNAL void rox_stack_push_int(CoreStack *stack, int value);

ROX_INTERNAL void rox_stack_push_double(CoreStack *stack, double value);

ROX_INTERNAL void rox_stack_push_boolean(CoreStack *stack, bool value);

ROX_INTERNAL void rox_stack_push_string_copy(CoreStack *stack, const char *value);

ROX_INTERNAL void rox_stack_push_string_ptr(CoreStack *stack, char *value);

ROX_INTERNAL void rox_stack_push_list(CoreStack *stack, RoxList *value);

ROX_INTERNAL void rox_stack_push_map(CoreStack *stack, RoxMap *value);

ROX_INTERNAL void rox_stack_push_dynamic_value(CoreStack *stack, RoxDynamicValue *value);

ROX_INTERNAL void rox_stack_push_null(CoreStack *stack);

ROX_INTERNAL void rox_stack_push_undefined(CoreStack *stack);

ROX_INTERNAL void rox_stack_push_item_copy(CoreStack *stack, StackItem *item);

/**
 * DON'T FORGET TO CALL THIS.
 * @param stack A NON-NULL pointer to the stack.
 */
ROX_INTERNAL void rox_stack_free(CoreStack *stack);

//
// Stack item contents type check.
//
// To check which type the stack item has one of the following functions should be called.
//

ROX_INTERNAL bool rox_stack_is_numeric(StackItem *item);

ROX_INTERNAL bool rox_stack_is_int(StackItem *item);

ROX_INTERNAL bool rox_stack_is_double(StackItem *item);

ROX_INTERNAL bool rox_stack_is_boolean(StackItem *item);

ROX_INTERNAL bool rox_stack_is_string(StackItem *item);

ROX_INTERNAL bool rox_stack_is_list(StackItem *item);

ROX_INTERNAL bool rox_stack_is_map(StackItem *item);

ROX_INTERNAL bool rox_stack_is_undefined(StackItem *item);

ROX_INTERNAL bool rox_stack_is_null(StackItem *item);

//
// Stack item contents retrieval.
//
// Once stack item type is determined, one of the following get_***_value functions should be
// called to obtain the actual value. NOTE: the returned value is a constant pointer to the
// stack item data, and it shouldn't be freed directly. Instead, rox_stack_free function should be called
// on the entire stack object.
//

ROX_INTERNAL int rox_stack_get_int(StackItem *item);

ROX_INTERNAL double rox_stack_get_double(StackItem *item);

ROX_INTERNAL double rox_stack_get_number(StackItem *item);

ROX_INTERNAL bool rox_stack_get_boolean(StackItem *item);

ROX_INTERNAL char *rox_stack_get_string(StackItem *item);

ROX_INTERNAL RoxList *rox_stack_get_list(StackItem *item);

ROX_INTERNAL RoxMap *rox_stack_get_map(StackItem *item);

ROX_INTERNAL RoxDynamicValue *rox_stack_get_value(StackItem *item);
