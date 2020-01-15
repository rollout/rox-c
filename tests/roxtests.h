#pragma once

#include <stdarg.h>
#include <check.h>
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

#define ROX_TEST_CASE(test) _rox_create_test_case(#test, test)
#define ROX_RUN_TESTS(...) _rox_run_tests(_rox_create_test_suite(__FILE__, __VA_ARGS__))
#define ROX_TEST_SUITE(...) int main(int argc, char *argv[]) { return ROX_RUN_TESTS(__VA_ARGS__); }