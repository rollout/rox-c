#include <check.h>

#include "roxx/stack.h"

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

Suite *stack_suite(void) {
    Suite *suite = suite_create("stack_suite");

    TCase *tcase = tcase_create("test_will_push_into_stack_string");
    tcase_add_test(tcase, test_will_push_into_stack_string);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_push_into_stack_integer");
    tcase_add_test(tcase, test_will_push_into_stack_integer);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_push_into_stack_double");
    tcase_add_test(tcase, test_will_push_into_stack_double);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_push_into_stack_boolean");
    tcase_add_test(tcase, test_will_push_into_stack_boolean);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_push_into_stack_null");
    tcase_add_test(tcase, test_will_push_into_stack_null);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_push_into_stack_integer_and_string");
    tcase_add_test(tcase, test_will_push_into_stack_integer_and_string);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("test_will_peek_from_stack");
    tcase_add_test(tcase, test_will_peek_from_stack);
    suite_add_tcase(suite, tcase);

    return suite;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *suite = stack_suite();
    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}
