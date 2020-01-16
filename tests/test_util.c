#include <check.h>
#include <stdlib.h>

#include "roxtests.h"
#include "util.h"

START_TEST (test_substring_start_offset_out_of_bounds) {
    char *str = str_substring("test", 5, 1);
    ck_assert_ptr_null(str);
}

END_TEST

START_TEST (test_substring_start_offset_plus_length_out_of_bounds) {
    ck_assert_ptr_null(str_substring("test", 0, 5));
    ck_assert_ptr_null(str_substring("test", 4, 1));
}

END_TEST

START_TEST (test_substring_whole_string) {
    char *str = str_substring("test", 0, 4);
    ck_assert_str_eq(str, "test");
    free(str);
}

END_TEST

START_TEST (test_substring_start_legnth_is_zero) {
    char *str = str_substring("test", 0, 0);
    ck_assert_str_eq(str, "");
    free(str);
}

END_TEST

START_TEST (test_substring_prefix) {
    char *str = str_substring("test", 0, 2);
    ck_assert_str_eq(str, "te");
    free(str);
}

END_TEST

START_TEST (test_substring_suffix) {
    char *str = str_substring("test", 2, 2);
    ck_assert_str_eq(str, "st");
    free(str);
}

END_TEST

START_TEST (test_substring_middle) {
    char *str = str_substring("test", 1, 2);
    ck_assert_str_eq(str, "es");
    free(str);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_substring_start_offset_out_of_bounds),
        ROX_TEST_CASE(test_substring_start_offset_plus_length_out_of_bounds),
        ROX_TEST_CASE(test_substring_whole_string),
        ROX_TEST_CASE(test_substring_start_legnth_is_zero),
        ROX_TEST_CASE(test_substring_prefix),
        ROX_TEST_CASE(test_substring_suffix),
        ROX_TEST_CASE(test_substring_middle)
);