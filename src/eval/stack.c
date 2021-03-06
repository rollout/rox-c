//
// linked list implementation of stack
//

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "collections.h"

struct CoreStack {
    StackItem *current;
    RoxList *all;
};

struct StackItem {
    RoxDynamicValue *value;
    StackItem *next;
};

static StackItem *_stack_item_create(RoxDynamicValue *value) {
    assert(value);
    // calloc sets all bytes to zeroes, so pointers must be NULLed by default.
    StackItem *item = (StackItem *) calloc(1, sizeof(StackItem));
    item->value = value;
    return item;
}

static void _stack_item_free(StackItem *item) {
    assert(item);
    rox_dynamic_value_free(item->value);
    free(item);
}

ROX_INTERNAL CoreStack *rox_stack_create() {
    CoreStack *stack = (CoreStack *) calloc(1, sizeof(CoreStack));
    stack->all = ROX_EMPTY_LIST;
    return stack;
}

ROX_INTERNAL void rox_stack_free(CoreStack *stack) {
    assert(stack);
    rox_list_free_cb(stack->all, (void (*)(void *)) &_stack_item_free);
    free(stack);
}

ROX_INTERNAL bool rox_stack_is_empty(CoreStack *stack) {
    assert(stack);
    return !stack->current;
}

ROX_INTERNAL void _stack_push(CoreStack *stack, StackItem *item) {
    assert(stack);
    assert(item);
    rox_list_add(stack->all, item);
    item->next = stack->current;
    stack->current = item;
}

ROX_INTERNAL void rox_stack_push_int(CoreStack *stack, int value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_int(value));
}

ROX_INTERNAL void rox_stack_push_double(CoreStack *stack, double value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_double(value));
}

ROX_INTERNAL void rox_stack_push_boolean(CoreStack *stack, bool value) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_boolean(value));
}

ROX_INTERNAL void rox_stack_push_string_copy(CoreStack *stack, const char *value) {
    assert(stack);
    assert(value);
    rox_stack_push_string_ptr(stack, mem_copy_str(value));
}

ROX_INTERNAL void rox_stack_push_string_ptr(CoreStack *stack, char *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_string_ptr(value));
}

ROX_INTERNAL void rox_stack_push_list(CoreStack *stack, RoxList *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_list(value));
}

ROX_INTERNAL void rox_stack_push_map(CoreStack *stack, RoxMap *value) {
    assert(stack);
    assert(value);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_map(value));
}

ROX_INTERNAL void rox_stack_push_dynamic_value(CoreStack *stack, RoxDynamicValue *value) {
    assert(stack);
    assert(value);
    _stack_push(stack, _stack_item_create(value));
}

ROX_INTERNAL void rox_stack_push_null(CoreStack *stack) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_null());
}

ROX_INTERNAL void rox_stack_push_undefined(CoreStack *stack) {
    assert(stack);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_undefined());
}

ROX_INTERNAL void rox_stack_push_item_copy(CoreStack *stack, StackItem *item) {
    assert(stack);
    assert(item);
    rox_stack_push_dynamic_value(stack, rox_dynamic_value_create_copy(item->value));
}

ROX_INTERNAL StackItem *rox_stack_pop(struct CoreStack *stack) {
    assert(stack);
    if (rox_stack_is_empty(stack))
        return NULL;
    struct StackItem *current = stack->current;
    stack->current = current->next;
    return current;
}

ROX_INTERNAL StackItem *rox_stack_peek(CoreStack *stack) {
    assert(stack);
    return stack->current;
}

ROX_INTERNAL bool rox_stack_is_numeric(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_int(item->value) ||
           rox_dynamic_value_is_double(item->value);
}

ROX_INTERNAL bool rox_stack_is_int(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_int(item->value);
}

ROX_INTERNAL bool rox_stack_is_double(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_double(item->value);
}

ROX_INTERNAL bool rox_stack_is_boolean(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_boolean(item->value);
}

ROX_INTERNAL bool rox_stack_is_string(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_string(item->value);
}

ROX_INTERNAL bool rox_stack_is_list(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_list(item->value);
}

ROX_INTERNAL bool rox_stack_is_map(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_map(item->value);
}

ROX_INTERNAL bool rox_stack_is_undefined(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_undefined(item->value);
}

ROX_INTERNAL bool rox_stack_is_null(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_null(item->value);
}

ROX_INTERNAL int rox_stack_get_int(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_int(item->value);
}

ROX_INTERNAL double rox_stack_get_double(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_double(item->value);
}

ROX_INTERNAL double rox_stack_get_number(StackItem *item) {
    assert(item);
    return rox_dynamic_value_is_double(item->value)
           ? rox_dynamic_value_get_double(item->value)
           : (double) rox_dynamic_value_get_int(item->value);
}

ROX_INTERNAL bool rox_stack_get_boolean(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_boolean(item->value);
}

ROX_INTERNAL char *rox_stack_get_string(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_string(item->value);
}

ROX_INTERNAL RoxList *rox_stack_get_list(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_list(item->value);
}

ROX_INTERNAL RoxMap *rox_stack_get_map(StackItem *item) {
    assert(item);
    return rox_dynamic_value_get_map(item->value);
}

ROX_INTERNAL RoxDynamicValue *rox_stack_get_value(StackItem *item) {
    assert(item);
    return item->value;
}