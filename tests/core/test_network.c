#include <check.h>
#include <assert.h>
#include <core/consts.h>
#include "roxtests.h"
#include "core/network.h"
#include "core/configuration.h"
#include "core/reporting.h"
#include "util.h"
#include "fixtures.h"

//
// Test Fixtures
//

typedef struct ROX_INTERNAL RequestTestContext {
    ConfigurationFetchedInvoker *invoker;
    ErrorReporter *reporter;
    SdkSettings *sdk_settings;
    RoxOptions *rox_options;
    DeviceProperties *device_properties;
    BUID *buid;
    RequestTestFixture *request;
    int times_invoker_called;
} RequestTestContext;


static void _test_configuration_fetched_handler(void *target, ConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    RequestTestContext *ctx = (RequestTestContext *) target;
    ++ctx->times_invoker_called;
}

static void _test_error_reporting_func(void *target,
                                       ErrorReporter *reporter,
                                       const char *file,
                                       int line,
                                       const char *fmt,
                                       va_list ars) {
    assert(reporter);
    assert(fmt);
    // Stub
}

static RequestTestContext *_request_test_context_create(HashTable *device_properties_map, const char *buid) {

    RequestTestContext *ctx = calloc(1, sizeof(RequestTestContext));

    ctx->invoker = configuration_fetched_invoker_create();
    configuration_fetched_invoker_register_handler(ctx->invoker, ctx, &_test_configuration_fetched_handler);

    ErrorReporterConfig reporter_config = {ctx, &_test_error_reporting_func};
    ctx->reporter = error_reporter_create(&reporter_config);
    ctx->request = request_test_fixture_create();

    ctx->sdk_settings = sdk_settings_create("test", "test");
    ctx->rox_options = rox_options_create();
    ctx->device_properties = device_properties_map
                             ? device_properties_create_from_map(ctx->sdk_settings, ctx->rox_options,
                                                                 device_properties_map)
                             : device_properties_create(ctx->sdk_settings, ctx->rox_options);

    ctx->buid = buid
                ? buid_create_dummy(buid)
                : buid_create(ctx->device_properties);

    return ctx;
}

static ConfigurationFetcher *_test_create_conf_fetcher(RequestTestContext *ctx) {
    assert(ctx);
    return configuration_fetcher_create(
            ctx->request->request,
            ctx->sdk_settings,
            ctx->device_properties,
            ctx->buid,
            ctx->invoker,
            ctx->reporter);
}

static ConfigurationFetcher *_test_create_conf_fetcher_roxy(RequestTestContext *ctx) {
    assert(ctx);
    return configuration_fetcher_create_roxy(
            ctx->request->request,
            ctx->device_properties,
            ctx->buid,
            ctx->invoker,
            ctx->reporter,
            "http://harta.com");
}

static void _request_test_context_free(RequestTestContext *ctx) {
    assert(ctx);
    configuration_fetched_invoker_free(ctx->invoker);
    error_reporter_free(ctx->reporter);
    request_test_fixture_free(ctx->request);
    rox_options_free(ctx->rox_options);
    buid_free(ctx->buid);
    device_properties_free(ctx->device_properties);
    sdk_settings_free(ctx->sdk_settings);
    free(ctx);
}

//
// ConfigurationFetcherTests
//

START_TEST (test_will_return_cdn_data_when_successful) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");
    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{\"a\": \"harti\"}";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(ctx->request->last_get_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_get_params), 1);
    rox_check_map_contains(ctx->request->last_get_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "a")->valuestring, "harti");
    ck_assert_int_eq(CONFIGURATION_SOURCE_CDN, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    configuration_fetch_result_free(result);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_with_exception) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");
    ctx->request->status_to_return_to_get = 0;
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_empty_response) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");
    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(0, ctx->request->times_post_sent);
    // TODO: log record to check
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_succeed_with_not_json_response) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");
    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{fdsadf/:";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(0, ctx->request->times_post_sent);
    // TODO: log record to check
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_cdn_fails_404_api_with_exception) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");
    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 0;
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(1, ctx->request->times_post_sent);
    // TODO: log record to check
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_api_data_when_cdn_fails_with_result_404_api_ok) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");

    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{\"result\": \"404\"}";
    ctx->request->status_to_return_to_post = 200;
    ctx->request->data_to_return_to_post = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->request->last_get_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_get_params), 1);
    rox_check_map_contains(ctx->request->last_get_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(ctx->request->last_post_uri, "https://x-api.rollout.io/device/get_configuration/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_post_params), 5);
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_APP_KEY.name, "123");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_API_VERSION.name, "4.0.0");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_BUID.name, "buid");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name,
                           "123/buid");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_API, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(1, ctx->request->times_post_sent);

    configuration_fetch_result_free(result);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_api_data_when_cdn_succeed_with_result200) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");

    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->request->last_get_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_get_params), 1);
    rox_check_map_contains(ctx->request->last_get_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_CDN, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(0, ctx->request->times_post_sent);

    configuration_fetch_result_free(result);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_api_data_when_cdn_fails_404_api_ok) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");

    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 200;
    ctx->request->data_to_return_to_post = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->request->last_get_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_get_params), 1);
    rox_check_map_contains(ctx->request->last_get_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(ctx->request->last_post_uri, "https://x-api.rollout.io/device/get_configuration/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->request->last_post_params), 5);
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_APP_KEY.name, "123");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_API_VERSION.name, "4.0.0");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_BUID.name, "buid");
    rox_check_map_contains(ctx->request->last_post_params, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name,
                           "123/buid");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_API, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(1, ctx->request->times_post_sent);

    configuration_fetch_result_free(result);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_data_when_both_not_found) {
    RequestTestContext *ctx = _request_test_context_create(
            ROX_MAP(
                    "app_key", ROX_COPY("123"),
                    "api_version", ROX_COPY("4.0.0"),
                    "distinct_id", ROX_COPY("id")
            ), "buid");

    ctx->request->status_to_return_to_get = 404;
    ctx->request->status_to_return_to_post = 404;

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->request->times_get_sent);
    ck_assert_int_eq(1, ctx->request->times_post_sent);

    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

//
// ConfigurationFetcherRoxyTests
//

START_TEST (test_will_return_data_when_successful) {
    RequestTestContext *ctx = _request_test_context_create(NULL, NULL);
    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = "{\"a\": \"harti\"}";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher_roxy(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "a")->valuestring, "harti");
    ck_assert_int_eq(CONFIGURATION_SOURCE_ROXY, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    configuration_fetch_result_free(result);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_roxy_fails_with_exception) {
    RequestTestContext *ctx = _request_test_context_create(NULL, NULL);
    ctx->request->status_to_return_to_get = 0;
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher_roxy(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_null_when_roxy_fails_with_http_status) {
    RequestTestContext *ctx = _request_test_context_create(NULL, NULL);
    ctx->request->status_to_return_to_get = 404;
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher_roxy(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
// ConfigurationFetcherRoxyTests
        ROX_TEST_CASE(test_will_return_cdn_data_when_successful),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_with_exception),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_empty_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_succeed_with_not_json_response),
        ROX_TEST_CASE(test_will_return_null_when_cdn_fails_404_api_with_exception),
        ROX_TEST_CASE(test_will_return_api_data_when_cdn_fails_with_result_404_api_ok),
        ROX_TEST_CASE(test_will_return_api_data_when_cdn_succeed_with_result200),
        ROX_TEST_CASE(test_will_return_api_data_when_cdn_fails_404_api_ok),
        ROX_TEST_CASE(test_will_return_null_data_when_both_not_found),
// ConfigurationFetcherTests
        ROX_TEST_CASE(test_will_return_data_when_successful),
        ROX_TEST_CASE(test_will_return_null_when_roxy_fails_with_exception),
        ROX_TEST_CASE(test_will_return_null_when_roxy_fails_with_http_status)
)
