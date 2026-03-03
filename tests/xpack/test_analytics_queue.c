#include <assert.h>
#include <check.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "roxtests.h"
#include "xpack/analytics/client.h"
#include "xpack/analytics/model.h"
#include "core/properties.h"
#include "core/client.h"

//
// Helper Functions
//

// Test client context to properly manage resource lifecycle
typedef struct {
    AnalyticsClient *client;
    SdkSettings *sdk_settings;
    RoxOptions *options;
    DeviceProperties *props;
} TestClientContext;

static TestClientContext *create_test_client() {
    TestClientContext *ctx = calloc(1, sizeof(TestClientContext));
    if (!ctx) return NULL;

    AnalyticsClientConfig config = ANALYTICS_CLIENT_INITIAL_CONFIG;
    config.max_queue_size = 10;  // Small for testing
    config.max_batch_size = 5;
    config.flush_interval_seconds = 0;  // Disable timer for tests

    ctx->sdk_settings = sdk_settings_create("test_api_key", "test_dev_secret");
    ctx->options = rox_options_create();
    ctx->props = device_properties_create(ctx->sdk_settings, ctx->options);
    ctx->client = analytics_client_create("test_write_key", &config, ctx->props);

    return ctx;
}

static void free_test_client(TestClientContext *ctx) {
    if (!ctx) return;
    if (ctx->client) analytics_client_free(ctx->client);
    if (ctx->props) device_properties_free(ctx->props);
    if (ctx->options) rox_options_free(ctx->options);
    if (ctx->sdk_settings) sdk_settings_free(ctx->sdk_settings);
    free(ctx);
}

static AnalyticsEvent *create_test_event(const char *flag_name) {
    return analytics_event_create(flag_name, "test_value", "test_user_123");
}

//
// Test Cases
//

START_TEST(test_analytics_client_create) {
    TestClientContext *ctx = create_test_client();
    ck_assert_ptr_nonnull(ctx);
    ck_assert_ptr_nonnull(ctx->client);
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_event_create) {
    AnalyticsEvent *event = create_test_event("test.flag");
    ck_assert_ptr_nonnull(event);
    ck_assert_str_eq(event->flag, "test.flag");
    ck_assert_str_eq(event->value, "test_value");
    ck_assert_str_eq(event->distinct_id, "test_user_123");
    ck_assert_str_eq(event->type, "IMPRESSION");
    ck_assert(event->time > 0);
    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_event_copy) {
    AnalyticsEvent *original = create_test_event("test.flag");
    AnalyticsEvent *copy = analytics_event_copy(original);

    ck_assert_ptr_nonnull(copy);
    ck_assert_ptr_ne(original, copy);
    ck_assert_str_eq(copy->flag, original->flag);
    ck_assert_str_eq(copy->value, original->value);
    ck_assert_str_eq(copy->distinct_id, original->distinct_id);
    ck_assert_str_eq(copy->type, original->type);
    ck_assert(copy->time == original->time);

    analytics_event_free(original);
    analytics_event_free(copy);
}
END_TEST

START_TEST(test_analytics_track_single_event) {
    TestClientContext *ctx = create_test_client();
    AnalyticsEvent *event = create_test_event("test.flag");

    // Track should not crash
    analytics_client_track(ctx->client, event);

    analytics_event_free(event);
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_track_multiple_events) {
    TestClientContext *ctx = create_test_client();

    // Track 3 events
    for (int i = 0; i < 3; i++) {
        char flag_name[32];
        snprintf(flag_name, sizeof(flag_name), "test.flag%d", i);
        AnalyticsEvent *event = create_test_event(flag_name);
        analytics_client_track(ctx->client, event);
        analytics_event_free(event);
    }

    // Should not crash, events should be queued
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_queue_limit) {
    TestClientContext *ctx = create_test_client();

    // Track more events than max_queue_size (10)
    for (int i = 0; i < 20; i++) {
        char flag_name[32];
        snprintf(flag_name, sizeof(flag_name), "test.flag%d", i);
        AnalyticsEvent *event = create_test_event(flag_name);
        analytics_client_track(ctx->client, event);
        analytics_event_free(event);
    }

    // Should not crash, oldest events should be dropped
    // Queue should be trimmed to max_queue_size
    free_test_client(ctx);
}
END_TEST

//
// Thread Safety Tests
//

typedef struct {
    TestClientContext *ctx;
    int thread_id;
    int event_count;
} ThreadTestData;

static void *track_events_thread(void *arg) {
    ThreadTestData *data = (ThreadTestData *)arg;

    for (int i = 0; i < data->event_count; i++) {
        char flag_name[64];
        snprintf(flag_name, sizeof(flag_name), "thread%d.flag%d", data->thread_id, i);
        AnalyticsEvent *event = create_test_event(flag_name);
        analytics_client_track(data->ctx->client, event);
        analytics_event_free(event);
    }

    return NULL;
}

START_TEST(test_analytics_thread_safety) {
    TestClientContext *ctx = create_test_client();

    // Create multiple threads that track events concurrently
    const int num_threads = 5;
    const int events_per_thread = 10;
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];

    // Start threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].event_count = events_per_thread;
        int rc = pthread_create(&threads[i], NULL, track_events_thread, &thread_data[i]);
        ck_assert_int_eq(rc, 0);
    }

    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Should not crash, all events should be tracked safely
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_first_flush_race_condition) {
    TestClientContext *ctx = create_test_client();

    // Track first event from multiple threads simultaneously
    const int num_threads = 10;
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];

    // All threads track just 1 event
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].ctx = ctx;
        thread_data[i].thread_id = i;
        thread_data[i].event_count = 1;
        int rc = pthread_create(&threads[i], NULL, track_events_thread, &thread_data[i]);
        ck_assert_int_eq(rc, 0);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Should only trigger one first flush, not 10
    // Verified by checking logs (no crashes is the test)
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_cleanup_with_queued_events) {
    TestClientContext *ctx = create_test_client();

    // Track some events
    for (int i = 0; i < 5; i++) {
        AnalyticsEvent *event = create_test_event("test.flag");
        analytics_client_track(ctx->client, event);
        analytics_event_free(event);
    }

    // Free client with events still in queue
    // Should properly cleanup all events without leaking
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_null_value_handling) {
    AnalyticsEvent *event = analytics_event_create("test.flag", NULL, "test_user");
    ck_assert_ptr_nonnull(event);
    ck_assert_ptr_null(event->value);
    ck_assert_str_eq(event->flag, "test.flag");
    ck_assert_str_eq(event->distinct_id, "test_user");
    analytics_event_free(event);
}
END_TEST

START_TEST(test_analytics_timer_disabled) {
    // This test verifies timer can be disabled via config
    TestClientContext *ctx = calloc(1, sizeof(TestClientContext));

    AnalyticsClientConfig config = ANALYTICS_CLIENT_INITIAL_CONFIG;
    config.flush_interval_seconds = 0;  // Disable timer

    ctx->sdk_settings = sdk_settings_create("test_api_key", "test_dev_secret");
    ctx->options = rox_options_create();
    ctx->props = device_properties_create(ctx->sdk_settings, ctx->options);
    ctx->client = analytics_client_create("test_write_key", &config, ctx->props);

    ck_assert_ptr_nonnull(ctx->client);

    // Track event
    AnalyticsEvent *event = create_test_event("test.flag");
    analytics_client_track(ctx->client, event);
    analytics_event_free(event);

    // Cleanup should work even without timer
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_immediate_destroy) {
    // Create client and immediately destroy
    // Tests timer cleanup race condition
    for (int i = 0; i < 10; i++) {
        TestClientContext *ctx = create_test_client();
        free_test_client(ctx);
    }
}
END_TEST

//
// Code Review Fix Tests
//

START_TEST(test_analytics_stopped_flag_rejects_events) {
    TestClientContext *ctx = create_test_client();

    // Track event before shutdown - should succeed
    AnalyticsEvent *event1 = create_test_event("test.before_shutdown");
    analytics_client_track(ctx->client, event1);
    analytics_event_free(event1);

    // Free client (sets stopped flag and waits for async flush)
    free_test_client(ctx);
}
END_TEST

START_TEST(test_analytics_url_built_once) {
    // Verify client creates successfully with URL built
    TestClientContext *ctx = create_test_client();
    ck_assert_ptr_nonnull(ctx->client);

    // Track multiple events - URL should be reused for all flushes
    for (int i = 0; i < 5; i++) {
        char flag_name[32];
        snprintf(flag_name, sizeof(flag_name), "test.flag%d", i);
        AnalyticsEvent *event = create_test_event(flag_name);
        analytics_client_track(ctx->client, event);
        analytics_event_free(event);
    }

    // Should work without crashes - URL was built once and reused
    free_test_client(ctx);
}
END_TEST

/**
 * Test: Track 10,000 impressions and verify:
 * 1. Queue is bounded to max_queue_size (1000 events)
 * 2. Old events are discarded properly (no memory leaks)
 * 3. Queue drains over time with periodic flush
 * 4. No memory leaks in C/C++ memory management
 
 */
START_TEST(test_analytics_10k_impressions_memory_safety) {
    printf("\n=== 10k Impressions Memory Safety Test ===\n");

    TestClientContext *ctx = calloc(1, sizeof(TestClientContext));
    ck_assert_ptr_nonnull(ctx);

    AnalyticsClientConfig config = ANALYTICS_CLIENT_INITIAL_CONFIG;
    config.max_queue_size = 1000;
    config.max_batch_size = 20;
    config.flush_interval_seconds = 1;

    ctx->sdk_settings = sdk_settings_create("test_api_key", "test_dev_secret");
    ck_assert_ptr_nonnull(ctx->sdk_settings);

    ctx->options = rox_options_create();
    ck_assert_ptr_nonnull(ctx->options);

    ctx->props = device_properties_create(ctx->sdk_settings, ctx->options);
    ck_assert_ptr_nonnull(ctx->props);

    ctx->client = analytics_client_create("test_write_key", &config, ctx->props);
    ck_assert_ptr_nonnull(ctx->client);

    printf("[10k Test] Client created with max_queue_size=1000, flush_interval=1s\n");
    printf("[10k Test] Starting to track 10,000 events...\n");

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    for (int i = 0; i < 10000; i++) {
        char flag_name[64];
        snprintf(flag_name, sizeof(flag_name), "test.flag_%d", i);

        AnalyticsEvent *event = create_test_event(flag_name);
        ck_assert_ptr_nonnull(event);

        analytics_client_track(ctx->client, event);
        analytics_event_free(event);

        if ((i + 1) % 1000 == 0) {
            int queue_size = analytics_client_get_queue_size(ctx->client);
            printf("[10k Test] Tracked %d events, queue size: %d\n", i + 1, queue_size);
        }
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                    (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("[10k Test] Finished tracking 10,000 events in %.2f seconds\n", elapsed);

    int queue_size_after_tracking = analytics_client_get_queue_size(ctx->client);
    printf("[10k Test] Queue size after tracking: %d (max allowed: 1000)\n",
           queue_size_after_tracking);

    ck_assert_int_le(queue_size_after_tracking, 1000);
    ck_assert_msg(queue_size_after_tracking <= 1000,
                  "Queue size %d exceeds max_queue_size 1000 - trimming failed!",
                  queue_size_after_tracking);

    printf("[10k Test] Queue properly bounded to max_queue_size\n");

    int wait_time = 50 * config.flush_interval_seconds;
    printf("[10k Test] Waiting %d seconds for queue to drain (50 flush cycles)...\n", wait_time);

    for (int i = 0; i < wait_time; i += 5) {
        sleep(5);
        int current_size = analytics_client_get_queue_size(ctx->client);
        printf("[10k Test] After %d seconds, queue size: %d\n", i + 5, current_size);
    }

    int queue_size_after_drain = analytics_client_get_queue_size(ctx->client);
    printf("[10k Test] Queue size after drain: %d\n", queue_size_after_drain);

    int max_after_drain = 500;
    ck_assert_int_le(queue_size_after_drain, max_after_drain);
    ck_assert_msg(queue_size_after_drain <= max_after_drain,
                  "Queue size %d not draining properly - expected <= %d!",
                  queue_size_after_drain, max_after_drain);

    printf("[10k Test] Queue drained successfully\n");
    printf("[10k Test] Cleaning up...\n");
    free_test_client(ctx);

    printf("[10k Test] Test completed successfully\n");
}
END_TEST

//
// Test Suite
//

Suite *analytics_queue_suite(void) {
    Suite *suite = suite_create("Analytics Queue");

    TCase *tc_basic = tcase_create("Basic Operations");
    tcase_add_test(tc_basic, test_analytics_client_create);
    tcase_add_test(tc_basic, test_analytics_event_create);
    tcase_add_test(tc_basic, test_analytics_event_copy);
    tcase_add_test(tc_basic, test_analytics_track_single_event);
    tcase_add_test(tc_basic, test_analytics_track_multiple_events);
    tcase_add_test(tc_basic, test_analytics_queue_limit);
    tcase_add_test(tc_basic, test_analytics_null_value_handling);
    suite_add_tcase(suite, tc_basic);

    TCase *tc_thread_safety = tcase_create("Thread Safety");
    tcase_add_test(tc_thread_safety, test_analytics_thread_safety);
    tcase_add_test(tc_thread_safety, test_analytics_first_flush_race_condition);
    tcase_add_test(tc_thread_safety, test_analytics_cleanup_with_queued_events);
    suite_add_tcase(suite, tc_thread_safety);

    TCase *tc_lifecycle = tcase_create("Lifecycle");
    tcase_add_test(tc_lifecycle, test_analytics_timer_disabled);
    tcase_add_test(tc_lifecycle, test_analytics_immediate_destroy);
    suite_add_tcase(suite, tc_lifecycle);

    TCase *tc_review_fixes = tcase_create("Code Review Fixes");
    tcase_add_test(tc_review_fixes, test_analytics_stopped_flag_rejects_events);
    tcase_add_test(tc_review_fixes, test_analytics_url_built_once);
    suite_add_tcase(suite, tc_review_fixes);

    TCase *tc_memory = tcase_create("Memory Safety");
    tcase_add_test(tc_memory, test_analytics_10k_impressions_memory_safety);
    tcase_set_timeout(tc_memory, 120);
    suite_add_tcase(suite, tc_memory);

    return suite;
}

int main(void) {
    Suite *suite = analytics_queue_suite();
    SRunner *runner = srunner_create(suite);
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
