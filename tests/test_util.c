#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC

#include <check.h>
#include <stdlib.h>
#include <pcre2.h>

#include "roxtests.h"
#include "util.h"

START_TEST (test_matches_empty_strings) {
    ck_assert(str_matches("", "", 0));
}

END_TEST

START_TEST (test_matches_equal_strings) {
    ck_assert(str_matches("undefined", "undefined", 0));
}

END_TEST

START_TEST (test_matches_ignore_case) {
    ck_assert(!str_matches("undefined", "UNDEFINED", 0));
    ck_assert(str_matches("undefined", "UNDEFINED", PCRE2_CASELESS));
}

END_TEST

START_TEST (test_matches_strings_pattern) {
    const char *pattern = "^\"((\\\\.)|[^\\\\\\\\\"])*\"$";
    ck_assert(str_matches("\"\"", pattern, 0));
    ck_assert(str_matches("\"test\"", pattern, 0));
    ck_assert(str_matches("\"TEST\"", pattern, 0));
    ck_assert(!str_matches("\"", pattern, 0));
    ck_assert(!str_matches("\"test", pattern, 0));
    ck_assert(!str_matches("t\"est\"", pattern, 0));
}

END_TEST

START_TEST (test_matches_numeric_pattern) {
    const char *pattern = "^[\\-]{0,1}\\d+[\\.]\\d+|[\\-]{0,1}\\d+$";
    ck_assert(!str_matches("\"\"", pattern, 0));
    ck_assert(!str_matches("\"0.1\"", pattern, 0));
    ck_assert(!str_matches("\"1\"", pattern, 0));
    ck_assert(str_matches("1", pattern, 0));
    ck_assert(str_matches("1.2", pattern, 0));
    ck_assert(str_matches("-1", pattern, 0));
    ck_assert(str_matches("-1.2", pattern, 0));
}

END_TEST

START_TEST (test_matches_bool_pattern) {
    const char *pattern = "^true|false$";
    ck_assert(!str_matches("\"true\"", pattern, 0));
    ck_assert(!str_matches("\"false\"", pattern, 0));
    ck_assert(!str_matches("\"TRUE\"", pattern, 0));
    ck_assert(!str_matches("\"FALSE\"", pattern, 0));
    ck_assert(!str_matches("\"true\"", pattern, 0));
    ck_assert(str_matches("true", pattern, PCRE2_CASELESS));
    ck_assert(str_matches("false", pattern, PCRE2_CASELESS));
    ck_assert(str_matches("TRUE", pattern, PCRE2_CASELESS));
    ck_assert(str_matches("FALSE", pattern, PCRE2_CASELESS));
}

END_TEST

START_TEST (test_index_of_empty_string) {
    ck_assert(str_index_of("", 'a') < 0);
}

END_TEST

START_TEST (test_index_of_not_found) {
    ck_assert(str_index_of("a", 'b') < 0);
}

END_TEST

START_TEST (test_index_of_single_character_string) {
    ck_assert(str_index_of("a", 'a') == 0);
}

END_TEST

START_TEST (test_index_of_first_occurrence) {
    ck_assert(str_index_of("abcabc", 'a') == 0);
    ck_assert(str_index_of("abc", 'b') == 1);
    ck_assert(str_index_of("abc", 'c') == 2);
}

END_TEST

START_TEST (test_str_to_double_nan) {
    ck_assert_ptr_null(str_to_double(""));
    ck_assert_ptr_null(str_to_double("a"));
    ck_assert_ptr_null(str_to_double("a123"));
}

END_TEST

START_TEST (test_str_to_double_valid_number) {
    char *strs[] = {"0", "1", "-1", "1.2", "-1.2", "123a", "-123a", "123.4a", "-123.4a"};
    double nums[] = {0, 1, -1, 1.2, -1.2, 123, -123, 123.4, -123.4};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        double *p = str_to_double(strs[i]);
        ck_assert_ptr_nonnull(p);
        ck_assert_double_eq(*p, nums[i]);
        free(p);
    }
}

END_TEST

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
// str_to_double
        ROX_TEST_CASE(test_str_to_double_nan),
        ROX_TEST_CASE(test_str_to_double_valid_number),

// str_matches
        ROX_TEST_CASE(test_matches_empty_strings),
        ROX_TEST_CASE(test_matches_equal_strings),
        ROX_TEST_CASE(test_matches_ignore_case),
        ROX_TEST_CASE(test_matches_strings_pattern),
        ROX_TEST_CASE(test_matches_numeric_pattern),
        ROX_TEST_CASE(test_matches_bool_pattern),

// str_index_of
        ROX_TEST_CASE(test_index_of_empty_string),
        ROX_TEST_CASE(test_index_of_not_found),
        ROX_TEST_CASE(test_index_of_single_character_string),
        ROX_TEST_CASE(test_index_of_first_occurrence),

// str_substring
        ROX_TEST_CASE(test_substring_start_offset_out_of_bounds),
        ROX_TEST_CASE(test_substring_start_offset_plus_length_out_of_bounds),
        ROX_TEST_CASE(test_substring_whole_string),
        ROX_TEST_CASE(test_substring_start_legnth_is_zero),
        ROX_TEST_CASE(test_substring_prefix),
        ROX_TEST_CASE(test_substring_suffix),
        ROX_TEST_CASE(test_substring_middle)
)