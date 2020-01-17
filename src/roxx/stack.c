//
// linked list implementation of stack
//

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "util.h"
#include "roxx/stack.h"

struct ROX_INTERNAL CoreStack {
    StackItem *first;
    StackItem *current;
};

struct ROX_INTERNAL StackItem {
    int *int_value;
    double *double_value;
    char *str_value;
    bool *bool_value;
    bool is_null;
    StackItem *next;
};

CoreStack *ROX_INTERNAL stack_create() {
    return (CoreStack *) calloc(1, sizeof(CoreStack));
}

void ROX_INTERNAL rox_stack_free(CoreStack *stack) {
    assert(stack);
    StackItem *item = stack->first;
    while (item) {
        StackItem *next = item->next;
        if (item->int_value) {
            free(item->int_value);
        }
        if (item->double_value) {
            free(item->double_value);
        }
        if (item->bool_value) {
            free(item->bool_value);
        }
        if (item->str_value) {
            free(item->str_value);
        }
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

StackItem *ROX_INTERNAL _create_stack_item() {
    // calloc sets all bytes to zeroes, so pointers must be NULLed by default.
    return (StackItem *) calloc(1, sizeof(StackItem));
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
    StackItem *item = _create_stack_item();
    item->int_value = mem_copy_int(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_double(CoreStack *stack, double value) {
    StackItem *item = _create_stack_item();
    item->double_value = mem_copy_double(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_boolean(CoreStack *stack, bool value) {
    StackItem *item = _create_stack_item();
    item->bool_value = mem_copy_bool(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_string(CoreStack *stack, const char *value) {
    StackItem *item = _create_stack_item();
    item->str_value = mem_copy_str(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_null(CoreStack *stack) {
    StackItem *item = _create_stack_item();
    item->is_null = true;
    _stack_push(stack, item);
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
    return item->int_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_double(StackItem *item) {
    assert(item);
    return item->double_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_boolean(StackItem *item) {
    assert(item);
    return item->bool_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_string(StackItem *item) {
    assert(item);
    return item->str_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_null(StackItem *item) {
    assert(item);
    return item->is_null;
}

int ROX_INTERNAL rox_stack_get_int(StackItem *item) {
    assert(item);
    assert(item->int_value);
    return *item->int_value;
}

double ROX_INTERNAL rox_stack_get_double(StackItem *item) {
    assert(item);
    assert(item->double_value);
    return *item->double_value;
}

bool ROX_INTERNAL rox_stack_get_boolean(StackItem *item) {
    assert(item);
    assert(item->bool_value);
    return *item->bool_value;
}

char *ROX_INTERNAL rox_stack_get_string(StackItem *item) {
    assert(item);
    assert(item->str_value);
    return item->str_value;
}
