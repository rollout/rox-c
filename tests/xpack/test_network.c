#include <check.h>
#include <assert.h>

#include "roxtests.h"
#include "xpack/network.h"
#include "util.h"

//
// DebouncerTests
//

static void _test_debouncer_func(void *target) {
    assert(target);
    int *counter = (int *) target;
    ++(*counter);
}

START_TEST (test_will_test_debouncer_called_after_interval) {
    int counter = 0;
    Debouncer *debouncer = debouncer_create(1000, &counter, &_test_debouncer_func);
    ck_assert_int_eq(0, counter);
    debouncer_invoke(debouncer);
    ck_assert_int_eq(0, counter);
    thread_sleep(500);
    ck_assert_int_eq(0, counter);
    thread_sleep(600);
    ck_assert_int_eq(1, counter);
    debouncer_free(debouncer);
}

END_TEST

START_TEST (test_will_test_debouncer_skip_double_invoke) {
    int counter = 0;
    Debouncer *debouncer = debouncer_create(1000, &counter, &_test_debouncer_func);
    ck_assert_int_eq(0, counter);
    debouncer_invoke(debouncer);
    ck_assert_int_eq(0, counter);
    thread_sleep(500);
    ck_assert_int_eq(0, counter);
    debouncer_invoke(debouncer);
    ck_assert_int_eq(0, counter);
    thread_sleep(600);
    ck_assert_int_eq(1, counter);
    thread_sleep(600);
    ck_assert_int_eq(1, counter);
    debouncer_free(debouncer);
}

END_TEST

START_TEST (test_will_test_debouncer_invoke_after_invoke) {
    int counter = 0;
    Debouncer *debouncer = debouncer_create(1000, &counter, &_test_debouncer_func);
    ck_assert_int_eq(0, counter);
    debouncer_invoke(debouncer);
    ck_assert_int_eq(0, counter);
    thread_sleep(1100);
    ck_assert_int_eq(1, counter);
    debouncer_invoke(debouncer);
    ck_assert_int_eq(1, counter);
    thread_sleep(800);
    ck_assert_int_eq(1, counter);
    thread_sleep(300);
    ck_assert_int_eq(2, counter);
    debouncer_free(debouncer);
}

END_TEST

START_TEST (test_will_awake_debouncer_thread) {
    double time = current_time_millis();
    int counter = 0;
    Debouncer *debouncer = debouncer_create(10000, &counter, &_test_debouncer_func);
    debouncer_invoke(debouncer);
    thread_sleep(300);
    debouncer_free(debouncer);
    ck_assert_int_eq(0, counter);
    double time_passed = current_time_millis() - time;
    ck_assert_int_lt(time_passed, 2000);
}

END_TEST

//
// StateSenderTests
//

START_TEST (test_will_call_cdn_succefuly) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_changes_for_flag) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_changes_for_custom_property) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_flag_order_not_important) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_custom_property_order_not_important) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_with_exception) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_empty_response) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_not_json_response) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_404_api_with_exception) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_api_data_when_cdn_succeed_with_result404apiok) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_apidata_when_cdn_fails_404_api_ok) {
    // TODO: implement!
}

END_TEST

START_TEST (test_will_return_null_data_when_both_not_found) {
    // TODO: implement!
}

END_TEST

ROX_TEST_SUITE(
// DebouncerTests
        ROX_TEST_CASE(test_will_test_debouncer_called_after_interval),
        ROX_TEST_CASE(test_will_test_debouncer_skip_double_invoke),
        ROX_TEST_CASE(test_will_test_debouncer_invoke_after_invoke),
        ROX_TEST_CASE(test_will_awake_debouncer_thread),
// StateSenderTests
        ROX_TEST_CASE(test_will_call_cdn_succefuly),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_changes_for_flag),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_changes_for_custom_property),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_flag_order_not_important),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_custom_property_order_not_important),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_with_exception),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_empty_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_not_json_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_404_api_with_exception),
        ROX_TEST_CASE(test_will_return_api_data_when_cdn_succeed_with_result404apiok),
        ROX_TEST_CASE(test_will_return_apidata_when_cdn_fails_404_api_ok),
        ROX_TEST_CASE(test_will_return_null_data_when_both_not_found)
)
