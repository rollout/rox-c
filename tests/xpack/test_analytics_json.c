#include <assert.h>
#include <check.h>
#include <string.h>
#include "roxtests.h"
#include "xpack/analytics/model.h"
#include "util.h"

//
// JSON Validation Tests
//
// Note: These tests verify the event structure, not the internal JSON serialization
// since that's a private function. We verify the structure is correct.
//

START_TEST(test_analytics_event_structure) {
    AnalyticsEvent *event = analytics_event_create(
        "feature.test_flag",
        "test_value",
        "user_12345");

    // Verify all fields are set
    ck_assert_ptr_nonnull(event);
    ck_assert_ptr_nonnull(event->flag);
    ck_assert_ptr_nonnull(event->value);
    ck_assert_ptr_nonnull(event->distinct_id);
    ck_assert_ptr_nonnull(event->type);

    // Verify field values
    ck_assert_str_eq(event->flag, "feature.test_flag");
    ck_assert_str_eq(event->value, "test_value");
    ck_assert_str_eq(event->distinct_id, "user_12345");
    ck_assert_str_eq(event->type, "IMPRESSION");

    // Verify timestamp is reasonable (not 0, not in future)
    ck_assert(event->time > 1000000000000.0);  // After year 2001
    ck_assert(event->time < 9999999999999.0);  // Before year 2286

    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_null_value) {
    AnalyticsEvent *event = analytics_event_create(
        "feature.test_flag",
        NULL,  // NULL value
        "user_12345");

    ck_assert_ptr_nonnull(event);
    ck_assert_ptr_null(event->value);  // Should handle NULL gracefully
    ck_assert_str_eq(event->flag, "feature.test_flag");
    ck_assert_str_eq(event->distinct_id, "user_12345");

    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_special_characters) {
    // Test with special characters that need JSON escaping
    AnalyticsEvent *event = analytics_event_create(
        "feature.\"quoted\"",
        "value with\nnewline\tand\ttabs",
        "user\\with\\backslashes");

    ck_assert_ptr_nonnull(event);
    ck_assert_str_eq(event->flag, "feature.\"quoted\"");
    ck_assert_str_eq(event->value, "value with\nnewline\tand\ttabs");
    ck_assert_str_eq(event->distinct_id, "user\\with\\backslashes");

    // cJSON should handle escaping automatically
    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_unicode) {
    // Test with Unicode characters
    AnalyticsEvent *event = analytics_event_create(
        "feature.测试",
        "值\xE2\x9C\x93",  // UTF-8 checkmark
        "用户_123");

    ck_assert_ptr_nonnull(event);
    ck_assert_str_eq(event->flag, "feature.测试");

    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_long_strings) {
    // Test with very long strings
    char long_flag[1024];
    char long_value[1024];
    char long_distinct_id[1024];

    memset(long_flag, 'a', sizeof(long_flag) - 1);
    long_flag[sizeof(long_flag) - 1] = '\0';

    memset(long_value, 'b', sizeof(long_value) - 1);
    long_value[sizeof(long_value) - 1] = '\0';

    memset(long_distinct_id, 'c', sizeof(long_distinct_id) - 1);
    long_distinct_id[sizeof(long_distinct_id) - 1] = '\0';

    AnalyticsEvent *event = analytics_event_create(
        long_flag,
        long_value,
        long_distinct_id);

    ck_assert_ptr_nonnull(event);
    ck_assert_str_eq(event->flag, long_flag);
    ck_assert_str_eq(event->value, long_value);
    ck_assert_str_eq(event->distinct_id, long_distinct_id);

    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_copy_preserves_fields) {
    AnalyticsEvent *original = analytics_event_create(
        "feature.original",
        "original_value",
        "user_original");

    AnalyticsEvent *copy = analytics_event_copy(original);

    // Verify copy has same data
    ck_assert_str_eq(copy->flag, original->flag);
    ck_assert_str_eq(copy->value, original->value);
    ck_assert_str_eq(copy->distinct_id, original->distinct_id);
    ck_assert_str_eq(copy->type, original->type);
    ck_assert(copy->time == original->time);

    // Verify they are separate memory
    ck_assert_ptr_ne(copy->flag, original->flag);
    ck_assert_ptr_ne(copy->value, original->value);
    ck_assert_ptr_ne(copy->distinct_id, original->distinct_id);
    ck_assert_ptr_ne(copy->type, original->type);

    // Modify copy, should not affect original
    free(copy->flag);
    copy->flag = mem_copy_str("modified.flag");

    ck_assert_str_eq(original->flag, "feature.original");
    ck_assert_str_eq(copy->flag, "modified.flag");

    analytics_event_free(original);
    analytics_event_free(copy);
}
END_TEST

START_TEST(test_analytics_event_type_always_impression) {
    // Verify type is always "IMPRESSION"
    AnalyticsEvent *event1 = analytics_event_create("flag1", "value1", "user1");
    AnalyticsEvent *event2 = analytics_event_create("flag2", NULL, "user2");
    AnalyticsEvent *event3 = analytics_event_create("flag3", "value3", "user3");

    ck_assert_str_eq(event1->type, "IMPRESSION");
    ck_assert_str_eq(event2->type, "IMPRESSION");
    ck_assert_str_eq(event3->type, "IMPRESSION");

    analytics_event_free(event1);
    analytics_event_free(event2);
    analytics_event_free(event3);
}
END_TEST

START_TEST(test_analytics_event_timestamp_increments) {
    // Verify each event gets a unique timestamp
    AnalyticsEvent *event1 = analytics_event_create("flag1", "value1", "user1");
    AnalyticsEvent *event2 = analytics_event_create("flag2", "value2", "user2");
    AnalyticsEvent *event3 = analytics_event_create("flag3", "value3", "user3");

    // Timestamps should be increasing or equal (same millisecond)
    ck_assert(event2->time >= event1->time);
    ck_assert(event3->time >= event2->time);

    analytics_event_free(event1);
    analytics_event_free(event2);
    analytics_event_free(event3);
}
END_TEST

START_TEST(test_analytics_event_distinct_id_placeholder) {
    // Test the placeholder distinct_id used when none is provided
    AnalyticsEvent *event = analytics_event_create(
        "test.flag",
        "test_value",
        "(null_distinct_id");  // Same as impression.c line 64

    ck_assert_ptr_nonnull(event);
    ck_assert_str_eq(event->distinct_id, "(null_distinct_id");

    analytics_event_free(event);
}
END_TEST

//
// Test Suite
//

Suite *analytics_json_suite(void) {
    Suite *suite = suite_create("Analytics JSON");

    TCase *tc_structure = tcase_create("Event Structure");
    tcase_add_test(tc_structure, test_analytics_event_structure);
    tcase_add_test(tc_structure, test_analytics_event_null_value);
    tcase_add_test(tc_structure, test_analytics_event_special_characters);
    tcase_add_test(tc_structure, test_analytics_event_unicode);
    tcase_add_test(tc_structure, test_analytics_event_long_strings);
    tcase_add_test(tc_structure, test_analytics_event_type_always_impression);
    tcase_add_test(tc_structure, test_analytics_event_timestamp_increments);
    tcase_add_test(tc_structure, test_analytics_event_distinct_id_placeholder);
    suite_add_tcase(suite, tc_structure);

    TCase *tc_copy = tcase_create("Event Copy");
    tcase_add_test(tc_copy, test_analytics_event_copy_preserves_fields);
    suite_add_tcase(suite, tc_copy);

    return suite;
}

int main(void) {
    Suite *suite = analytics_json_suite();
    SRunner *runner = srunner_create(suite);
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
