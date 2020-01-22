//
// linked list implementation of stack
//

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <collectc/list.h>
#include <collectc/hashtable.h>
#include "util.h"
#include "stack.h"

struct ROX_INTERNAL CoreStack {
    StackItem *first;
    StackItem *current;
};

struct ROX_INTERNAL StackItem {
    int *int_value;
    double *double_value;
    char *str_value;
    List *list_value;
    HashTable *map_value;
    bool is_true;
    bool is_false;
    bool is_null;
    bool is_undefined;
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
        if (item->int_value) {
            free(item->int_value);
        }
        if (item->double_value) {
            free(item->double_value);
        }
        if (item->str_value) {
            free(item->str_value);
        }
        // NOTE: we don't manage lists and maps,
        // so not destroying them here.
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
    assert(stack);
    StackItem *item = _create_stack_item();
    item->int_value = mem_copy_int(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_double(CoreStack *stack, double value) {
    assert(stack);
    StackItem *item = _create_stack_item();
    item->double_value = mem_copy_double(value);
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_boolean(CoreStack *stack, bool value) {
    assert(stack);
    StackItem *item = _create_stack_item();
    item->is_true = value;
    item->is_false = !value;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_string_copy(CoreStack *stack, const char *value) {
    assert(stack);
    assert(value);
    rox_stack_push_string_ptr(stack, mem_copy_str(value));
}

void ROX_INTERNAL rox_stack_push_string_ptr(CoreStack *stack, char *value) {
    assert(stack);
    assert(value);
    StackItem *item = _create_stack_item();
    item->str_value = value;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_list(CoreStack *stack, List *value) {
    assert(stack);
    assert(value);
    StackItem *item = _create_stack_item();
    item->list_value = value;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_map(CoreStack *stack, HashTable *value) {
    assert(stack);
    assert(value);
    StackItem *item = _create_stack_item();
    item->map_value = value;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_null(CoreStack *stack) {
    assert(stack);
    StackItem *item = _create_stack_item();
    item->is_null = true;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_undefined(CoreStack *stack) {
    assert(stack);
    StackItem *item = _create_stack_item();
    item->is_undefined = true;
    _stack_push(stack, item);
}

void ROX_INTERNAL rox_stack_push_item_copy(CoreStack *stack, StackItem *item) {
    assert(stack);
    assert(item);
    StackItem *copy = _create_stack_item();
    if (item->int_value) {
        copy->int_value = mem_copy_int(*item->int_value);
    }
    if (item->double_value) {
        copy->double_value = mem_copy_double(*item->double_value);
    }
    if (item->str_value) {
        copy->str_value = mem_copy_str(item->str_value);
    }
    copy->list_value = item->list_value;
    copy->map_value = item->map_value;
    copy->is_undefined = item->is_undefined;
    copy->is_null = item->is_null;
    copy->is_true = item->is_true;
    copy->is_false = item->is_false;
    _stack_push(stack, copy);
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
    return item->is_true || item->is_false;
}

bool ROX_INTERNAL rox_stack_is_string(StackItem *item) {
    assert(item);
    return item->str_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_list(StackItem *item) {
    assert(item);
    return item->list_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_map(StackItem *item) {
    assert(item);
    return item->map_value != NULL;
}

bool ROX_INTERNAL rox_stack_is_undefined(StackItem *item) {
    assert(item);
    return item->is_undefined;
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
    assert(item->is_true || item->is_false);
    return item->is_true;
}

char *ROX_INTERNAL rox_stack_get_string(StackItem *item) {
    assert(item);
    assert(item->str_value);
    return item->str_value;
}

List *ROX_INTERNAL rox_stack_get_list(StackItem *item) {
    assert(item);
    assert(item->list_value);
    return item->list_value;
}

HashTable *ROX_INTERNAL rox_stack_get_map(StackItem *item) {
    assert(item);
    assert(item->map_value);
    return item->map_value;
}
