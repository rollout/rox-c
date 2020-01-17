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
    ck_assert_ptr_null(mem_str_to_double(""));
    ck_assert_ptr_null(mem_str_to_double("a"));
    ck_assert_ptr_null(mem_str_to_double("a123"));
}

END_TEST

START_TEST (test_str_to_double_valid_number) {
    char *strs[] = {"0", "1", "-1", "1.2", "-1.2", "123a", "-123a", "123.4a", "-123.4a"};
    double nums[] = {0, 1, -1, 1.2, -1.2, 123, -123, 123.4, -123.4};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        double *p = mem_str_to_double(strs[i]);
        ck_assert_ptr_nonnull(p);
        ck_assert_double_eq(*p, nums[i]);
        free(p);
    }
}

END_TEST

START_TEST (test_substring_start_offset_out_of_bounds) {
    char *str = mem_str_substring("test", 5, 1);
    ck_assert_ptr_null(str);
}

END_TEST

START_TEST (test_substring_start_offset_plus_length_out_of_bounds) {
    ck_assert_ptr_null(mem_str_substring("test", 0, 5));
    ck_assert_ptr_null(mem_str_substring("test", 4, 1));
}

END_TEST

START_TEST (test_substring_whole_string) {
    char *str = mem_str_substring("test", 0, 4);
    ck_assert_str_eq(str, "test");
    free(str);
}

END_TEST

START_TEST (test_substring_start_legnth_is_zero) {
    char *str = mem_str_substring("test", 0, 0);
    ck_assert_str_eq(str, "");
    free(str);
}

END_TEST

START_TEST (test_substring_prefix) {
    char *str = mem_str_substring("test", 0, 2);
    ck_assert_str_eq(str, "te");
    free(str);
}

END_TEST

START_TEST (test_substring_suffix) {
    char *str = mem_str_substring("test", 2, 2);
    ck_assert_str_eq(str, "st");
    free(str);
}

END_TEST

START_TEST (test_substring_middle) {
    char *str = mem_str_substring("test", 1, 2);
    ck_assert_str_eq(str, "es");
    free(str);
}

END_TEST

START_TEST (test_replace) {

    // NOTE: mem_str_replace tests are taken from here
    // https://gist.github.com/dhess/975639/bb91cd552c0a92306b8ef49b417c6796f67036ce

    const char *s1 = 0;
    const char *s2 = 0;
    const char *s3 = 0;
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = "";
    s2 = 0;
    s3 = 0;
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = 0;
    s2 = "";
    s3 = 0;
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = "";
    s2 = "";
    s3 = 0;
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = 0;
    s2 = 0;
    s3 = "";
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = "";
    s2 = 0;
    s3 = "";
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = 0;
    s2 = "";
    s3 = "";
    ck_assert(mem_str_replace(s1, s2, s3) == 0);

    s1 = "";
    s2 = "";
    s3 = "";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);

    s1 = "abc";
    s2 = "";
    s3 = "xyz";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);

    s1 = "";
    s2 = "abc";
    s3 = "xyz";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);

    s1 = "abc";
    s2 = "def";
    s3 = "xyz";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);

    s1 = "ab";
    s2 = "abc";
    s3 = "xyz";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);

    s1 = "abc";
    s2 = "abc";
    s3 = "xyz";

    char *s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyz") == 0);
    ck_assert(s4 != s1);
    free(s4);

    s1 = "abc";
    s2 = "a";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyzbc") == 0);
    free(s4);

    s1 = "abc";
    s2 = "b";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "axyzc") == 0);
    free(s4);

    s1 = "abc";
    s2 = "c";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abxyz") == 0);
    free(s4);

    s1 = "aba";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyza") == 0);
    free(s4);

    s1 = "bbc";
    s2 = "bc";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "bxyz") == 0);
    free(s4);

    s1 = "a";
    s2 = "a";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyz") == 0);
    free(s4);

    s1 = "ab";
    s2 = "a";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyzb") == 0);
    free(s4);

    s1 = "ab";
    s2 = "b";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "axyz") == 0);
    free(s4);

    s1 = "abbc";
    s2 = "ab";
    s3 = "x";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xbc") == 0);
    free(s4);

    s1 = "abcc";
    s2 = "bc";
    s3 = "x";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "axc") == 0);
    free(s4);

    s1 = "dccd";
    s2 = "cd";
    s3 = "x";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "dcx") == 0);
    free(s4);

    s1 = "abab";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyzxyz") == 0);
    free(s4);

    s1 = "abcab";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyzcxyz") == 0);
    free(s4);

    s1 = "abcabc";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "xyzcxyzc") == 0);
    free(s4);

    s1 = "cabcab";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "cxyzcxyz") == 0);
    free(s4);

    s1 = "cabcabc";
    s2 = "ab";
    s3 = "xyz";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "cxyzcxyzc") == 0);
    free(s4);

    s1 = "abc";
    s2 = "ab";
    s3 = "ab";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abc") == 0);
    free(s4);

    s1 = "abc";
    s2 = "bc";
    s3 = "bc";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abc") == 0);
    free(s4);

    s1 = "abcc";
    s2 = "abc";
    s3 = "ab";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abc") == 0);
    free(s4);

    s1 = "abccc";
    s2 = "bc";
    s3 = "b";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abcc") == 0);
    free(s4);

    s1 = "abccc";
    s2 = "cc";
    s3 = "c";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abcc") == 0);
    free(s4);

    s1 = "abcd";
    s2 = "a";
    s3 = "";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "bcd") == 0);
    free(s4);

    s1 = "abcd";
    s2 = "bc";
    s3 = "";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "ad") == 0);
    free(s4);

    s1 = "abcd";
    s2 = "d";
    s3 = "";
    s4 = mem_str_replace(s1, s2, s3);
    ck_assert(strcmp(s4, "abc") == 0);
    free(s4);

    s1 = "";
    s2 = "";
    s3 = "abc";
    ck_assert(mem_str_replace(s1, s2, s3) == s1);
}

END_TEST

ROX_TEST_SUITE(
// mem_str_to_double
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

// mem_str_substring
        ROX_TEST_CASE(test_substring_start_offset_out_of_bounds),
        ROX_TEST_CASE(test_substring_start_offset_plus_length_out_of_bounds),
        ROX_TEST_CASE(test_substring_whole_string),
        ROX_TEST_CASE(test_substring_start_legnth_is_zero),
        ROX_TEST_CASE(test_substring_prefix),
        ROX_TEST_CASE(test_substring_suffix),
        ROX_TEST_CASE(test_substring_middle),

// mem_str_replace
        ROX_TEST_CASE(test_replace)
)