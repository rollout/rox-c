#pragma once

#include <stdarg.h>
#include <check.h>
#include <core/properties.h>
#include "roxapi.h"

Suite *ROX_INTERNAL _rox_create_test_suite(char *name, ...) {
    va_list args;
            va_start(args, name);

    Suite *suite = suite_create(name);

    TCase *test_case = va_arg(args, TCase*);
    while (test_case != NULL) {
        suite_add_tcase(suite, test_case);
        test_case = va_arg(args, TCase*);
    };
            va_end(args);
    return suite;
}

TCase *ROX_INTERNAL _rox_create_test_case(char *name, const TTest *test) {
    TCase *test_case = tcase_create(name);
    tcase_add_test(test_case, test);
    return test_case;
}

int ROX_INTERNAL _rox_run_tests(Suite *suite) {
    int number_failed;
    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}

void ROX_INTERNAL rox_check_and_free(char *str, const char *expected_value) {
    ck_assert_str_eq(str, expected_value);
    if (str) {
        free(str);
    }
}

void ROX_INTERNAL rox_check_map_contains(HashTable *map, const char *key, const char *expected_value) {
    char *actual_value;
    ck_assert_int_eq(hashtable_get(map, (void *) key, (void **) &actual_value), CC_OK);
    ck_assert_str_eq(actual_value, expected_value);
}

#define ROX_TEST_CASE(test) _rox_create_test_case(#test, test)
#define ROX_RUN_TESTS(...) _rox_run_tests(_rox_create_test_suite(__FILE__, __VA_ARGS__, NULL))
#define ROX_TEST_SUITE(...) int main(int argc, char *argv[]) { return ROX_RUN_TESTS(__VA_ARGS__); }

void rox_check_prop_str(CustomProperty *prop, const char *expected_value, Context *context) {
    DynamicValue *value = custom_property_get_value(prop, context);
    ck_assert_ptr_nonnull(value);
    ck_assert_str_eq(dynamic_value_get_string(value), expected_value);
    dynamic_value_free(value);
}

void rox_check_prop_int(CustomProperty *prop, int expected_value, Context *context) {
    DynamicValue *value = custom_property_get_value(prop, context);
    ck_assert_ptr_nonnull(value);
    ck_assert_int_eq(dynamic_value_get_int(value), expected_value);
    dynamic_value_free(value);
}

void rox_check_prop_double(CustomProperty *prop, double expected_value, Context *context) {
    DynamicValue *value = custom_property_get_value(prop, context);
    ck_assert_ptr_nonnull(value);
    ck_assert_double_eq(dynamic_value_get_double(value), expected_value);
    dynamic_value_free(value);
}

void rox_check_prop_bool(CustomProperty *prop, bool expected_value, Context *context) {
    DynamicValue *value = custom_property_get_value(prop, context);
    ck_assert_ptr_nonnull(value);
    ck_assert_int_eq(dynamic_value_get_boolean(value), expected_value);
    dynamic_value_free(value);
}