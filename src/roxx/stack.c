//
// linked list implementation of stack
//

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <collectc/list.h>
#include <collectc/hashtable.h>
#include "stack.h"
#include "util.h"
#include "dynamic.h"

struct ROX_INTERNAL CoreStack {
    StackItem *first;
    StackItem *current;
};

struct ROX_INTERNAL StackItem {
    DynamicValue *value;
    StackItem *next;
};

CoreStack *ROX_INTERNAL rox_stack_create() {
    return (CoreStack *) calloc(1, sizeof(CoreStack));
}

void ROX_INTERNAL rox_stack_free(CoreStack *stack) {
    assert(stack);
    StackItem *item = stack->first;
    while (item) {
        StackItem *next = item->next;
        dynamic_value_free(item->value);
        free(item);
        item = next;
    }
    stack->current = NULL;
    free(stack);
}

bool ROX_INTERNAL rox_stack_is_empty(CoreStack *stack) {
    assert(stack);
    return !stack->current;
}

StackItem *ROX_INTERNAL _create_stack_item(DynamicValue *value) {
    // calloc sets all bytes to zeroes, so pointers must be NULLed by default.
    StackItem *item = (StackItem *) calloc(1, sizeof(StackItem));
    item->value = value;
    return item;
}

void ROX_INTERNAL _stack_push(CoreStack *stack, StackItem *item) {
    assert(stack);
    assert(item);
    if (!stack->first) {
        stack->first = item;
    }
    item->next = stack->current;
    stack->current = item;
}

void ROX_INTERNAL rox_stack_push_int(CoreStack *stack, int value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_int(value));
}

void ROX_INTERNAL rox_stack_push_double(CoreStack *stack, double value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_double(value));
}

void ROX_INTERNAL rox_stack_push_boolean(CoreStack *stack, bool value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_boolean(value));
}

void ROX_INTERNAL rox_stack_push_string_copy(CoreStack *stack, const char *value) {
    assert(stack);
    assert(value);
    rox_stack_push_string_ptr(stack, mem_copy_str(value));
}

void ROX_INTERNAL rox_stack_push_string_ptr(CoreStack *stack, char *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_string_ptr(value));
}

void ROX_INTERNAL rox_stack_push_list(CoreStack *stack, List *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_list(value));
}

void ROX_INTERNAL rox_stack_push_map(CoreStack *stack, HashTable *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_map(value));
}

void ROX_INTERNAL rox_stack_push_dynamic_value(CoreStack *stack, DynamicValue *value) {
    assert(stack);
    assert(value);
    _stack_push(stack, _create_stack_item(value));
}

void ROX_INTERNAL rox_stack_push_null(CoreStack *stack) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_null());
}

void ROX_INTERNAL rox_stack_push_undefined(CoreStack *stack) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_undefined());
}

void ROX_INTERNAL rox_stack_push_item_copy(CoreStack *stack, StackItem *item) {
    assert(stack);
    assert(item);
    rox_stack_push_dynamic_value(stack, dynamic_value_create_copy(item->value));
}

StackItem *ROX_INTERNAL rox_stack_pop(struct CoreStack *stack) {
    assert(stack);
    if (rox_stack_is_empty(stack))
        return NULL;
    struct StackItem *current = stack->current;
    stack->current = current->next;
    return current;
}

StackItem *ROX_INTERNAL rox_stack_peek(CoreStack *stack) {
    assert(stack);
    return stack->current;
}

bool ROX_INTERNAL rox_stack_is_int(StackItem *item) {
    assert(item);
    return dynamic_value_is_int(item->value);
}

bool ROX_INTERNAL rox_stack_is_double(StackItem *item) {
    assert(item);
    return dynamic_value_is_double(item->value);
}

bool ROX_INTERNAL rox_stack_is_boolean(StackItem *item) {
    assert(item);
    return dynamic_value_is_boolean(item->value);
}

bool ROX_INTERNAL rox_stack_is_string(StackItem *item) {
    assert(item);
    return dynamic_value_is_string(item->value);
}

bool ROX_INTERNAL rox_stack_is_list(StackItem *item) {
    assert(item);
    return dynamic_value_is_list(item->value);
}

bool ROX_INTERNAL rox_stack_is_map(StackItem *item) {
    assert(item);
    return dynamic_value_is_map(item->value);
}

bool ROX_INTERNAL rox_stack_is_undefined(StackItem *item) {
    assert(item);
    return dynamic_value_is_undefined(item->value);
}

bool ROX_INTERNAL rox_stack_is_null(StackItem *item) {
    assert(item);
    return dynamic_value_is_null(item->value);
}

int ROX_INTERNAL rox_stack_get_int(StackItem *item) {
    assert(item);
    return dynamic_value_get_int(item->value);
}

double ROX_INTERNAL rox_stack_get_double(StackItem *item) {
    assert(item);
    return dynamic_value_get_double(item->value);
}

bool ROX_INTERNAL rox_stack_get_boolean(StackItem *item) {
    assert(item);
    return dynamic_value_get_boolean(item->value);
}

char *ROX_INTERNAL rox_stack_get_string(StackItem *item) {
    assert(item);
    return dynamic_value_get_string(item->value);
}

List *ROX_INTERNAL rox_stack_get_list(StackItem *item) {
    assert(item);
    return dynamic_value_get_list(item->value);
}

HashTable *ROX_INTERNAL rox_stack_get_map(StackItem *item) {
    assert(item);
    return dynamic_value_get_map(item->value);
}
