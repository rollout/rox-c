// TODO: put parser functions here

#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "core/entities.h"
#include "roxx/parser.h"
#include "roxx/stack.h"

struct ROX_INTERNAL EvaluationResult {
    int *int_value;
    double *double_value;
    char *str_value;
    bool *bool_value;
};

EvaluationResult *ROX_INTERNAL _create_result_from_stack_item(StackItem *item) {
    assert(item);

    EvaluationResult *result = calloc(1, sizeof(EvaluationResult));

    if (stack_is_null(item)) {
        result->bool_value = false;
    }

    if (stack_is_boolean(item)) {
        result->bool_value = malloc(sizeof(bool));
        *result->bool_value = stack_get_boolean(item);
        result->str_value = mem_copy_str(*result->bool_value
                                         ? FLAG_TRUE_VALUE
                                         : FLAG_FALSE_VALUE);
    }

    if (stack_is_int(item)) {
        int value = stack_get_int(item);
        result->int_value = mem_copy_int(value);
        result->double_value = mem_copy_double(value);
    }

    if (stack_is_double(item)) {
        double value = stack_get_double(item);
        result->int_value = mem_copy_int((int) value);
        result->double_value = mem_copy_double(value);
    }

    if (stack_is_string(item)) {
        char *value = stack_get_string(item);
        result->int_value = str_to_int(value);
        result->double_value = str_to_double(value);
    }

    return result;
}

int *ROX_INTERNAL result_get_int(EvaluationResult *result) {
    return result->int_value;
}

double *ROX_INTERNAL result_get_double(EvaluationResult *result) {
    assert(result);
    return result->double_value;
}

bool *ROX_INTERNAL result_get_boolean(EvaluationResult *result) {
    assert(result);
    return result->bool_value;
}

char *ROX_INTERNAL result_get_string(EvaluationResult *result) {
    assert(result);
    return result->str_value;
}

void ROX_INTERNAL result_free(EvaluationResult *result) {
    assert(result);
    if (result->int_value) {
        free(result->int_value);
    }
    if (result->double_value) {
        free(result->double_value);
    }
    if (result->bool_value) {
        free(result->bool_value);
    }
    if (result->str_value) {
        free(result->str_value);
    }
    free(result);
}
