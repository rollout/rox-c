#pragma once

#include <stdbool.h>
#include <collectc/list.h>
#include <collectc/hashtable.h>
#include <dynamic.h>
#include "roxapi.h"

typedef struct ROX_INTERNAL CoreStack CoreStack;
typedef struct ROX_INTERNAL StackItem StackItem;

//
// Stack manipulation.
//
// In ROX stack may contain various data types such as ints, floats, strings and so on.
// Here we define functions to create and use stack, and destroy it once it is not under use.
//

CoreStack *ROX_INTERNAL rox_stack_create();

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, stack item otherwise.
 */
StackItem *ROX_INTERNAL rox_stack_pop(CoreStack *stack);

/**
 * @param stack A NON-NULL pointer.
 * @return NULL if stack is empty, current stack item otherwise.
 */
StackItem *ROX_INTERNAL rox_stack_peek(CoreStack *stack);

bool ROX_INTERNAL rox_stack_is_empty(CoreStack *stack);

void ROX_INTERNAL rox_stack_push_int(CoreStack *stack, int value);

void ROX_INTERNAL rox_stack_push_double(CoreStack *stack, double value);

void ROX_INTERNAL rox_stack_push_boolean(CoreStack *stack, bool value);

void ROX_INTERNAL rox_stack_push_string_copy(CoreStack *stack, const char *value);

void ROX_INTERNAL rox_stack_push_string_ptr(CoreStack *stack, char *value);

void ROX_INTERNAL rox_stack_push_list(CoreStack *stack, List *value);

void ROX_INTERNAL rox_stack_push_map(CoreStack *stack, HashTable *value);

void ROX_INTERNAL rox_stack_push_dynamic_value(CoreStack *stack, DynamicValue* value);

void ROX_INTERNAL rox_stack_push_null(CoreStack *stack);

void ROX_INTERNAL rox_stack_push_undefined(CoreStack *stack);

void ROX_INTERNAL rox_stack_push_item_copy(CoreStack *stack, StackItem *item);

/**
 * DON'T FORGET TO CALL THIS.
 * @param stack A NON-NULL pointer to the stack.
 */
void ROX_INTERNAL rox_stack_free(CoreStack *stack);

//
// Stack item contents type check.
//
// To check which type the stack item has one of the following functions should be called.
//

bool ROX_INTERNAL rox_stack_is_int(StackItem *item);

bool ROX_INTERNAL rox_stack_is_double(StackItem *item);

bool ROX_INTERNAL rox_stack_is_boolean(StackItem *item);

bool ROX_INTERNAL rox_stack_is_string(StackItem *item);

bool ROX_INTERNAL rox_stack_is_list(StackItem *item);

bool ROX_INTERNAL rox_stack_is_map(StackItem *item);

bool ROX_INTERNAL rox_stack_is_undefined(StackItem *item);

bool ROX_INTERNAL rox_stack_is_null(StackItem *item);

//
// Stack item contents retrieval.
//
// Once stack item type is determined, one of the following get_***_value functions should be
// called to obtain the actual value. NOTE: the returned value is a constant pointer to the
// stack item data, and it shouldn't be freed directly. Instead, rox_stack_free function should be called
// on the entire stack object.
//

int ROX_INTERNAL rox_stack_get_int(StackItem *item);

double ROX_INTERNAL rox_stack_get_double(StackItem *item);

bool ROX_INTERNAL rox_stack_get_boolean(StackItem *item);

char *ROX_INTERNAL rox_stack_get_string(StackItem *item);

List *ROX_INTERNAL rox_stack_get_list(StackItem *item);

HashTable *ROX_INTERNAL rox_stack_get_map(StackItem *item);
