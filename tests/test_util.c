#include <check.h>
#include <stdlib.h>
#include <pcre2.h>

#include "roxtests.h"
#include "util.h"
#include "collections.h"

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

START_TEST (test_str_to_int_valid_number) {
    char *strs[] = {"0", "1", "-1"};
    int nums[] = {0, 1, -1};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        int *p = mem_str_to_int(strs[i]);
        ck_assert_ptr_nonnull(p);
        ck_assert_int_eq(*p, nums[i]);
        free(p);
    }
}

END_TEST

START_TEST (test_str_to_int_floating_point) {
    char *strs[] = {"-3.5", "0.0", "3.5"};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        int *p = mem_str_to_int(strs[i]);
        ck_assert_ptr_null(p);
    }
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
    char *strs[] = {"0", "1", "-1", "1.2", "-1.2"};
    double nums[] = {0, 1, -1, 1.2, -1.2, 123, -123, 123.4, -123.4};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        double *p = mem_str_to_double(strs[i]);
        ck_assert_ptr_nonnull(p);
        ck_assert_double_eq(*p, nums[i]);
        free(p);
    }
}

END_TEST

START_TEST (test_str_to_double_invalid_number) {
    char *strs[] = {"", "a", "1a", "a1", "123a", "-123a", "123.4a", "-123.4a", "-a234"};
    for (int i = 0, n = sizeof(strs) / sizeof(char *); i < n; ++i) {
        double *p = mem_str_to_double(strs[i]);
        ck_assert_ptr_null(p);
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

START_TEST (test_base64_encode) {
    char *str = mem_base64_encode("test");
    ck_assert_str_eq(str, "dGVzdA==");
    free(str);
}

END_TEST

START_TEST (test_base64_decode) {
    rox_check_and_free(mem_base64_decode_str("dGVzdA=="), "test");
    rox_check_and_free(mem_base64_decode_str("Ymx1ZQ=="), "blue");
    rox_check_and_free(mem_base64_decode_str("Qmx1ZQ=="), "Blue");
}

END_TEST

START_TEST (test_md5_rfc1321_test_suite) {

    // https://tools.ietf.org/html/rfc1321

    char *md5 = mem_md5_str("");
    ck_assert_str_eq(md5, "d41d8cd98f00b204e9800998ecf8427e");
    free(md5);

    md5 = mem_md5_str("a");
    ck_assert_str_eq(md5, "0cc175b9c0f1b6a831c399e269772661");
    free(md5);

    md5 = mem_md5_str("abc");
    ck_assert_str_eq(md5, "900150983cd24fb0d6963f7d28e17f72");
    free(md5);

    md5 = mem_md5_str("message digest");
    ck_assert_str_eq(md5, "f96b697d7cb7938d525a2f31aaf161d0");
    free(md5);

    md5 = mem_md5_str("abcdefghijklmnopqrstuvwxyz");
    ck_assert_str_eq(md5, "c3fcd3d76192e4007dfb496cca67e13b");
    free(md5);

    md5 = mem_md5_str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    ck_assert_str_eq(md5, "d174ab98d277d9f5a5611c2c9f419d9f");
    free(md5);

    md5 = mem_md5_str("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    ck_assert_str_eq(md5, "57edf4a22be3c955ac49da2e2107b67a");
    free(md5);
}

END_TEST

START_TEST (test_json_serialization) {
    cJSON *json = ROX_JSON_OBJECT(
            "string", ROX_JSON_STRING("test"),
            "int", ROX_JSON_INT(1099),
            "double", ROX_JSON_DOUBLE(1099.99));
    char *json_string = ROX_JSON_SERIALIZE(json);
    ck_assert_str_eq("{\"string\":\"test\",\"int\":1099,\"double\":1099.99}", json_string);
    free(json_string);
    cJSON_Delete(json);
}

START_TEST (test_string_in_list) {
    RoxList *list = rox_list_create();
    ck_assert(!str_in_list("", list));
    rox_list_add(list, "one");
    rox_list_add(list, "two");
    rox_list_add(list, "three");
    ck_assert(!str_in_list("ONE", list));
    ck_assert(str_in_list("one", list));
    ck_assert(str_in_list(strstr("sub-three", "three"), list));
    ck_assert(!str_in_list("", list));
    rox_list_free(list);
}

END_TEST

START_TEST (test_string_join) {
    RoxList *list = rox_list_create();
    rox_check_and_free(mem_str_join("", list), "");
    rox_check_and_free(mem_str_join("|", list), "");

    rox_list_add(list, "");
    rox_check_and_free(mem_str_join("", list), "");
    rox_check_and_free(mem_str_join("|", list), "");

    rox_list_add(list, "");
    rox_check_and_free(mem_str_join("", list), "");
    rox_check_and_free(mem_str_join("|", list), "|");

    rox_list_remove_all(list);
    rox_list_add(list, "one");
    rox_check_and_free(mem_str_join("", list), "one");
    rox_check_and_free(mem_str_join("|", list), "one");

    rox_list_add(list, "two");
    rox_check_and_free(mem_str_join("", list), "onetwo");
    rox_check_and_free(mem_str_join("|", list), "one|two");

    rox_list_add(list, "three");
    rox_check_and_free(mem_str_join("", list), "onetwothree");
    rox_check_and_free(mem_str_join("|", list), "one|two|three");
    rox_check_and_free(mem_str_join(" AND ", list), "one AND two AND three");

    rox_list_free(list);
}

END_TEST

START_TEST (test_string_uppercase) {
    rox_check_and_free(str_to_upper(mem_copy_str("")), "");
    rox_check_and_free(str_to_upper(mem_copy_str("A")), "A");
    rox_check_and_free(str_to_upper(mem_copy_str("a")), "A");
    rox_check_and_free(str_to_upper(mem_copy_str("The quick brown fox jumps over the lazy dog")),
                       "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
    rox_check_and_free(str_to_upper(mem_copy_str("The Quick Brown Fox Jumps Over The Lazy Dog")),
                       "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
    rox_check_and_free(str_to_upper(mem_copy_str("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG")),
                       "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG");
}

END_TEST

START_TEST (test_build_url) {
    rox_check_and_free(mem_build_url("", ""), "");
    rox_check_and_free(mem_build_url("/", "/"), "/");
    rox_check_and_free(mem_build_url("test", ""), "test");
    rox_check_and_free(mem_build_url("test/", "/"), "test/");
    rox_check_and_free(mem_build_url("test/", "/me"), "test/me");
}

END_TEST

static bool check_str_lists_equal(RoxList *l1, RoxList *l2) {
    bool result = str_list_equals(l1, l2);
    rox_list_free(l1);
    rox_list_free(l2);
    return result;
}

START_TEST (test_str_list_equals) {
    ck_assert(check_str_lists_equal(ROX_EMPTY_LIST, ROX_EMPTY_LIST));
    ck_assert(check_str_lists_equal(ROX_LIST(""), ROX_LIST("")));
    ck_assert(check_str_lists_equal(ROX_LIST("1"), ROX_LIST("1")));
    ck_assert(!check_str_lists_equal(ROX_LIST("1"), ROX_LIST("2")));
    ck_assert(check_str_lists_equal(ROX_LIST("1", "2", "3"), ROX_LIST("1", "2", "3")));
    ck_assert(!check_str_lists_equal(ROX_LIST("1", "2", "3"), ROX_LIST("3", "2", "1")));
}

END_TEST

ROX_TEST_SUITE(
// mem_str_to_int
        ROX_TEST_CASE(test_str_to_int_floating_point),
        ROX_TEST_CASE(test_str_to_int_valid_number),

// mem_str_to_double
        ROX_TEST_CASE(test_str_to_double_nan),
        ROX_TEST_CASE(test_str_to_double_valid_number),
        ROX_TEST_CASE(test_str_to_double_invalid_number),

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

// mem_base64
        ROX_TEST_CASE(test_base64_encode),
        ROX_TEST_CASE(test_base64_decode),

// mem_md5_str
        ROX_TEST_CASE(test_md5_rfc1321_test_suite),

// ROX_JSON_XXX
        ROX_TEST_CASE(test_json_serialization),

// str_in_list
        ROX_TEST_CASE(test_string_in_list),

// mem_str_join
        ROX_TEST_CASE(test_string_join),

// str_to_upper
        ROX_TEST_CASE(test_string_uppercase),

// build_url
        ROX_TEST_CASE(test_build_url),

// str_list_equals
        ROX_TEST_CASE(test_str_list_equals)
)