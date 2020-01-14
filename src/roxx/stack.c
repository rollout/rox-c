//
// linked list implementation of stack
//

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "roxx/stack.h"

struct ROX_INTERNAL CoreStack {
    StackItem *first;
    StackItem *current;
};

struct ROX_INTERNAL StackItem {
    int *int_value;
    float *float_value;
    double *double_value;
    char *str_value;
    bool *bool_value;
    StackItem *next;
};

CoreStack *ROX_INTERNAL stack_create() {
    return (CoreStack *) calloc(1, sizeof(CoreStack));
}

void ROX_INTERNAL stack_free(CoreStack *stack) {
    assert(stack);
    StackItem *item = stack->first;
    while (item) {
        StackItem *next = item->next;
        if (item->int_value) {
            free(item->int_value);
        }
        if (item->float_value) {
            free(item->float_value);
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

bool ROX_INTERNAL stack_is_empty(CoreStack *stack) {
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

void ROX_INTERNAL stack_push_int(CoreStack *root, int value) {
    StackItem *item = _create_stack_item();
    item->int_value = malloc(sizeof(value));
    *item->int_value = value;
    _stack_push(root, item);
}

void ROX_INTERNAL stack_push_float(CoreStack *root, float value) {
    StackItem *item = _create_stack_item();
    item->float_value = malloc(sizeof(value));
    *item->float_value = value;
    _stack_push(root, item);
}

void ROX_INTERNAL stack_push_double(CoreStack *root, double value) {
    StackItem *item = _create_stack_item();
    item->double_value = malloc(sizeof(value));
    *item->double_value = value;
    _stack_push(root, item);
}

void ROX_INTERNAL stack_push_boolean(CoreStack *root, bool value) {
    StackItem *item = _create_stack_item();
    item->bool_value = malloc(sizeof(value));
    *item->bool_value = value;
    _stack_push(root, item);
}

void ROX_INTERNAL stack_push_string(CoreStack *root, const char *const value) {
    StackItem *item = _create_stack_item();
    unsigned int length = strlen(value);
    item->str_value = malloc((length + 1) * sizeof(char));
    strncpy_s(item->str_value, length + 1, value, length + 1);
    _stack_push(root, item);
}

StackItem *ROX_INTERNAL stack_pop(struct CoreStack *stack) {
    assert(stack);
    if (stack_is_empty(stack))
        return NULL;
    struct StackItem *current = stack->current;
    stack->current = current->next;
    return current;
}

StackItem *ROX_INTERNAL stack_peek(CoreStack *stack) {
    assert(stack);
    return stack->current;
}

bool ROX_INTERNAL is_int_value(StackItem *item) {
    assert(item);
    return item->int_value != NULL;
}

bool ROX_INTERNAL is_float_value(StackItem *item) {
    assert(item);
    return item->float_value != NULL;
}

bool ROX_INTERNAL is_double_value(StackItem *item) {
    assert(item);
    return item->double_value != NULL;
}

bool ROX_INTERNAL is_boolean_value(StackItem *item) {
    assert(item);
    return item->bool_value != NULL;
}

bool ROX_INTERNAL is_string_value(StackItem *item) {
    assert(item);
    return item->str_value != NULL;
}

int ROX_INTERNAL get_int_value(StackItem *item) {
    assert(item);
    assert(item->int_value);
    return *item->int_value;
}

float ROX_INTERNAL get_float_value(StackItem *item) {
    assert(item);
    assert(item->float_value);
    return *item->float_value;
}

double ROX_INTERNAL get_double_value(StackItem *item) {
    assert(item);
    assert(item->double_value);
    return *item->double_value;
}

bool ROX_INTERNAL get_boolean_value(StackItem *item) {
    assert(item);
    assert(item->bool_value);
    return *item->bool_value;
}

char *ROX_INTERNAL get_string_value(StackItem *item) {
    assert(item);
    assert(item->str_value);
    return item->str_value;
}
