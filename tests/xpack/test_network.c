#include <check.h>
#include <assert.h>
#include <core/consts.h>

#include "roxtests.h"
#include "xpack/network.h"
#include "util.h"
#include "fixtures.h"

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

typedef struct ROX_INTERNAL StateSenderTestContext {
    RequestTestFixture *request;
    SdkSettings *sdk_settings;
    RoxOptions *rox_options;
    DeviceProperties *device_properties;
    FlagRepository *flag_repo;
    CustomPropertyRepository *cp_repo;
    LoggingTestFixture *logging;
    StateSender *sender;
} StateSenderTestContext;

static StateSenderTestContext *_state_sender_test_context_create() {

    StateSenderTestContext *ctx = calloc(1, sizeof(StateSenderTestContext));

    ctx->request = request_test_fixture_create();
    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{\"result\": \"200\"}";

    ctx->sdk_settings = sdk_settings_create("123", "shh...");
    ctx->rox_options = rox_options_create();
    ctx->flag_repo = flag_repository_create();
    ctx->cp_repo = custom_property_repository_create();
    ctx->device_properties = device_properties_create_from_map(
            ctx->sdk_settings, ctx->rox_options,
            ROX_MAP(
                    "platform", mem_copy_str(".net"),
                    "devModeSecret", mem_copy_str(ctx->sdk_settings->dev_mode_secret),
                    "app_key", mem_copy_str(ctx->sdk_settings->api_key),
                    "api_version", mem_copy_str("4.0.0")
            ));

    ctx->logging = logging_test_fixture_create(RoxLogLevelDebug);
    ctx->sender = state_sender_create(ctx->request->request, ctx->device_properties, ctx->flag_repo, ctx->cp_repo);

    return ctx;
}

static void _state_sender_test_context_add_string_property(
        StateSenderTestContext *ctx, const char *name, const char *value) {
    custom_property_repository_add_custom_property(
            ctx->cp_repo, custom_property_create_using_value(name,
                                                             &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                             dynamic_value_create_string_copy(
                                                                     value)));
}

static void _state_sender_test_context_add_double_property(
        StateSenderTestContext *ctx, const char *name, double value) {
    custom_property_repository_add_custom_property(
            ctx->cp_repo, custom_property_create_using_value(name,
                                                             &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                                                             dynamic_value_create_double(
                                                                     value)));
}

static void _state_sender_test_context_free(StateSenderTestContext *ctx) {
    assert(ctx);
    state_sender_free(ctx->sender);
    logging_test_fixture_free(ctx->logging);
    device_properties_free(ctx->device_properties);
    flag_repository_free(ctx->flag_repo);
    custom_property_repository_free(ctx->cp_repo);
    rox_options_free(ctx->rox_options);
    sdk_settings_free(ctx->sdk_settings);
    request_test_fixture_free(ctx->request);
    free(ctx);
}

static void check_map_value(HashTable *map, const char *key, const char *expected_value) {
    char *actual_value;
    ck_assert_int_eq(hashtable_get(map, (void *) key, (void **) &actual_value), CC_OK);
    ck_assert_str_eq(expected_value, actual_value);
}

static void _validate_request_params(StateSenderTestContext *ctx) {
    HashTable *params = ctx->request->last_post_params;
    check_map_value(params, ROX_PROPERTY_TYPE_PLATFORM.name, ".net");
    check_map_value(params, ROX_PROPERTY_TYPE_FEATURE_FLAGS.name,
                    "[{\"name\":\"flag\",\"defaultValue\":\"false\",\"options\":[\"false\",\"true\"]}]");
    check_map_value(params, ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES.name,
                    "[{\"name\":\"id\",\"type\":\"string\",\"externalType\":\"String\"}]");
    check_map_value(params, ROX_PROPERTY_TYPE_REMOTE_VARIABLES.name, "[]");
    check_map_value(params, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "shh...");
}

START_TEST (test_will_call_cdn_successfully) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag1");

    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/C973890080E2E1074002239A8F093068");

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_changes_for_flag) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag1");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/C973890080E2E1074002239A8F093068");

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag2");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 2);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/F81295A07A291D828AA7BFFCCD9DA0B7");

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_changes_for_custom_property) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();

    _state_sender_test_context_add_string_property(ctx, "cp1", "true");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/08C5B082527CE9B45B6AC79A52B7804E");

    _state_sender_test_context_add_double_property(ctx, "cp2", 20);
    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 2);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/91E9221FB21ED170B50D73B285A315C1");

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_flag_order_not_important) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag2");
    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag1");

    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/F81295A07A291D828AA7BFFCCD9DA0B7");

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_call_only_cdn_state_md5_custom_property_order_not_important) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();

    _state_sender_test_context_add_string_property(ctx, "cp1", "1111");
    _state_sender_test_context_add_string_property(ctx, "cp2", "2222");

    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/0D07300D3F83F344E7C472E3EF2ECCF0");

    _state_sender_test_context_free(ctx);

    ctx = _state_sender_test_context_create();

    _state_sender_test_context_add_string_property(ctx, "cp2", "2222");
    _state_sender_test_context_add_string_property(ctx, "cp1", "1111");

    state_sender_send(ctx->sender);
    logging_test_fixture_check_no_errors(ctx->logging);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/0D07300D3F83F344E7C472E3EF2ECCF0");

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_with_exception) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->status_to_return_to_get = 0;

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Failed to send state");

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/CF09F3555C13395B5F2DE947450F6FA9");
    ck_assert_int_eq(ctx->request->times_post_sent, 0);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_empty_response) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->data_to_return_to_get = "";

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Failed to send state");

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/CF09F3555C13395B5F2DE947450F6FA9");
    ck_assert_int_eq(ctx->request->times_post_sent, 0);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_not_json_response) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->data_to_return_to_get = "fdsafdas{";

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Failed to send state");

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/CF09F3555C13395B5F2DE947450F6FA9");
    ck_assert_int_eq(ctx->request->times_post_sent, 0);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_404_api_with_exception) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 0;

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    _state_sender_test_context_add_string_property(ctx, "id", "1111");
    state_sender_send(ctx->sender);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Failed to send state");

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/9351683310939C0C53BB3EE38E6B99FC");
    ck_assert_int_eq(ctx->request->times_post_sent, 1);
    ck_assert_str_eq(ctx->request->last_post_uri,
                     "https://x-api.rollout.io/device/update_state_store/123/9351683310939C0C53BB3EE38E6B99FC");

    _validate_request_params(ctx);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_api_data_when_cdn_succeed_with_result_404_api_ok) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->data_to_return_to_get = "{\"result\": \"404\"}";
    ctx->request->status_to_return_to_post = 200;

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    _state_sender_test_context_add_string_property(ctx, "id", "1111");
    state_sender_send(ctx->sender);

    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelDebug, "Failed to send state");
    logging_test_fixture_check_no_messages(ctx->logging, RoxLogLevelError);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/9351683310939C0C53BB3EE38E6B99FC");
    ck_assert_int_eq(ctx->request->times_post_sent, 1);
    ck_assert_str_eq(ctx->request->last_post_uri,
                     "https://x-api.rollout.io/device/update_state_store/123/9351683310939C0C53BB3EE38E6B99FC");

    _validate_request_params(ctx);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_apidata_when_cdn_fails_404_api_ok) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 200;

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    _state_sender_test_context_add_string_property(ctx, "id", "1111");
    state_sender_send(ctx->sender);

    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelDebug, "Failed to send state");
    logging_test_fixture_check_no_messages(ctx->logging, RoxLogLevelError);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://statestore.rollout.io/123/9351683310939C0C53BB3EE38E6B99FC");
    ck_assert_int_eq(ctx->request->times_post_sent, 1);
    ck_assert_str_eq(ctx->request->last_post_uri,
                     "https://x-api.rollout.io/device/update_state_store/123/9351683310939C0C53BB3EE38E6B99FC");

    _validate_request_params(ctx);

    _state_sender_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_data_when_both_not_found) {
    StateSenderTestContext *ctx = _state_sender_test_context_create();
    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 404;

    flag_repository_add_flag(ctx->flag_repo, variant_create_flag(), "flag");
    state_sender_send(ctx->sender);

    ck_assert_int_eq(ctx->request->times_get_sent, 1);
    ck_assert_int_eq(ctx->request->times_post_sent, 1);

    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Failed to send state");

    _state_sender_test_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
// DebouncerTests
        ROX_TEST_CASE(test_will_test_debouncer_called_after_interval),
        ROX_TEST_CASE(test_will_test_debouncer_skip_double_invoke),
        ROX_TEST_CASE(test_will_test_debouncer_invoke_after_invoke),
        ROX_TEST_CASE(test_will_awake_debouncer_thread),
// StateSenderTests
        ROX_TEST_CASE(test_will_call_cdn_successfully),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_changes_for_flag),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_changes_for_custom_property),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_flag_order_not_important),
        ROX_TEST_CASE(test_will_call_only_cdn_state_md5_custom_property_order_not_important),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_with_exception),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_empty_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_not_json_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_404_api_with_exception),
        ROX_TEST_CASE(test_will_return_api_data_when_cdn_succeed_with_result_404_api_ok),
        ROX_TEST_CASE(test_will_return_apidata_when_cdn_fails_404_api_ok),
        ROX_TEST_CASE(test_will_return_null_data_when_both_not_found)
)
