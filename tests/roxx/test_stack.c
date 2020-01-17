#include <check.h>

#include "roxx/stack.h"
#include "roxtests.h"

START_TEST (test_will_push_into_stack_string) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_string(stack, "stringTest");

    StackItem *popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_string(popped_item));
    ck_assert_str_eq(stack_get_string(popped_item), "stringTest");

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_push_into_stack_integer) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_int(stack, 5);

    StackItem *popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_int(popped_item));
    ck_assert_int_eq(stack_get_int(popped_item), 5);

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_push_into_stack_double) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_double(stack, 5.5);

    StackItem *popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_double(popped_item));
    ck_assert_double_eq(stack_get_double(popped_item), 5.5);

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_push_into_stack_boolean) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_boolean(stack, true);
    stack_push_boolean(stack, false);

    StackItem *popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_boolean(popped_item));
    ck_assert(!stack_get_boolean(popped_item));

    popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_boolean(popped_item));
    ck_assert(stack_get_boolean(popped_item));

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_push_into_stack_null) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_null(stack);
    stack_push_int(stack, 1);

    StackItem *popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_int(popped_item));
    ck_assert(!stack_is_null(popped_item));

    popped_item = stack_pop(stack);
    fail_if(!popped_item);
    ck_assert(stack_is_null(popped_item));
    ck_assert(!stack_is_int(popped_item));

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_push_into_stack_integer_and_string) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_int(stack, 5);
    stack_push_string(stack, "testString");

    StackItem *first = stack_pop(stack);
    StackItem *second = stack_pop(stack);

    fail_if(!first);
    fail_if(!second);

    ck_assert(stack_is_string(first));
    ck_assert_str_eq(stack_get_string(first), "testString");

    ck_assert(stack_is_int(second));
    ck_assert_int_eq(stack_get_int(second), 5);

    ck_assert(stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

START_TEST (test_will_peek_from_stack) {

    CoreStack *stack = stack_create();
    fail_if(!stack, "Could not crate stack");
    stack_push_int(stack, 5);
    stack_push_string(stack, "testString");

    StackItem *first = stack_peek(stack);
    StackItem *second = stack_pop(stack);

    fail_if(!first);
    fail_if(!second);

    ck_assert(stack_is_string(first));
    ck_assert_str_eq(stack_get_string(first), "testString");

    ck_assert(stack_is_string(second));
    ck_assert_str_eq(stack_get_string(first), "testString");

    ck_assert(!stack_is_empty(stack));

    stack_free(stack);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_will_push_into_stack_string),
        ROX_TEST_CASE(test_will_push_into_stack_integer),
        ROX_TEST_CASE(test_will_push_into_stack_double),
        ROX_TEST_CASE(test_will_push_into_stack_boolean),
        ROX_TEST_CASE(test_will_push_into_stack_null),
        ROX_TEST_CASE(test_will_push_into_stack_integer_and_string),
        ROX_TEST_CASE(test_will_peek_from_stack))