#include <check.h>
#include <assert.h>
#include <core/consts.h>
#include "roxtests.h"
#include "core/network.h"
#include "core/configuration.h"
#include "core/reporting.h"
#include "util.h"

//
// Test Fixtures
//

typedef struct ROX_INTERNAL RequestTestContext {
    ConfigurationFetchedInvoker *invoker;
    ErrorReporter *reporter;
    Request *request;
    SdkSettings *sdk_settings;
    RoxOptions *rox_options;
    DeviceProperties *device_properties;
    BUID *buid;
    int status_to_return_to_get_request;
    const char *data_to_return_to_get_request;
    int status_to_return_to_post_request;
    const char *data_to_return_to_post_request;
    int times_invoker_called;
    int times_get_request_sent;
    char *get_request_uri;
    HashTable *get_request_params;
    int times_post_request_sent;
    char *post_request_uri;
    HashTable *post_request_params;
} RequestTestContext;

static HttpResponseMessage *_test_request_send_get_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data);
    RequestTestContext *ctx = (RequestTestContext *) target;
    ++ctx->times_get_request_sent;
    ctx->get_request_uri = mem_copy_str(data->url);
    ctx->get_request_params = mem_deep_copy_str_value_map(data->params);
    if (!ctx->status_to_return_to_get_request) {
        return NULL;
    }
    return response_message_create(
            ctx->status_to_return_to_get_request,
            ctx->data_to_return_to_get_request
            ? mem_copy_str(ctx->data_to_return_to_get_request)
            : NULL);
}

static HttpResponseMessage *_test_request_send_post_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data);
    RequestTestContext *ctx = (RequestTestContext *) target;
    ++ctx->times_post_request_sent;
    ctx->post_request_uri = mem_copy_str(data->url);
    ctx->post_request_params = mem_deep_copy_str_value_map(data->params);
    if (!ctx->status_to_return_to_post_request) {
        return NULL;
    }
    return response_message_create(
            ctx->status_to_return_to_post_request,
            ctx->data_to_return_to_post_request
            ? mem_copy_str(ctx->data_to_return_to_post_request)
            : NULL);
}

static void _test_configuration_fetched_handler(void *target, ConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    RequestTestContext *ctx = (RequestTestContext *) target;
    ++ctx->times_invoker_called;
}

static void _test_error_reporting_func(void *target, ErrorReporter *reporter, const char *fmt, va_list args) {
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

    RequestConfig request_config = {ctx, _test_request_send_get_func, &_test_request_send_post_func, NULL};
    ctx->request = request_create(&request_config);

    ctx->sdk_settings = calloc(1, sizeof(SdkSettings));
    ctx->sdk_settings->api_key = mem_copy_str("test");
    ctx->sdk_settings->dev_mode_secret = mem_copy_str("test");
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
            ctx->request,
            ctx->sdk_settings,
            ctx->device_properties,
            ctx->buid,
            ctx->invoker,
            ctx->reporter);
}

static ConfigurationFetcher *_test_create_conf_fetcher_roxy(RequestTestContext *ctx) {
    assert(ctx);
    return configuration_fetcher_create_roxy(
            ctx->request,
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
    request_free(ctx->request);
    rox_options_free(ctx->rox_options);
    buid_free(ctx->buid);
    device_properties_free(ctx->device_properties);
    free(ctx->sdk_settings->api_key);
    free(ctx->sdk_settings->dev_mode_secret);
    free(ctx->sdk_settings);
    if (ctx->get_request_uri) {
        free(ctx->get_request_uri);
    }
    if (ctx->get_request_params) {
        rox_map_free_with_values(ctx->get_request_params);
    }
    if (ctx->post_request_params) {
        rox_map_free_with_values(ctx->post_request_params);
    }
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
    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "{\"a\": \"harti\"}";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);
    ck_assert_str_eq(ctx->get_request_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->get_request_params), 1);
    rox_check_map_contains(ctx->get_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
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
    ctx->status_to_return_to_get_request = 0;
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
    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(0, ctx->times_post_request_sent);
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
    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "{fdsadf/:";
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(0, ctx->times_post_request_sent);
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
    ctx->status_to_return_to_get_request = 404;
    ctx->status_to_return_to_post_request = 0;
    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(1, ctx->times_post_request_sent);
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

    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "{\"result\": \"404\"}";
    ctx->status_to_return_to_post_request = 200;
    ctx->data_to_return_to_post_request = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->get_request_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->get_request_params), 1);
    rox_check_map_contains(ctx->get_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(ctx->post_request_uri, "https://x-api.rollout.io/device/get_configuration/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->post_request_params), 5);
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_APP_KEY.name, "123");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_API_VERSION.name, "4.0.0");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_BUID.name, "buid");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name, "123/buid");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_API, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(1, ctx->times_post_request_sent);

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

    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->get_request_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->get_request_params), 1);
    rox_check_map_contains(ctx->get_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_CDN, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(0, ctx->times_post_request_sent);

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

    ctx->status_to_return_to_get_request = 404;
    ctx->status_to_return_to_post_request = 200;
    ctx->data_to_return_to_post_request = "{\"harto\": \"a\"}";

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_nonnull(result);

    ck_assert_str_eq(ctx->get_request_uri, "https://conf.rollout.io/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->get_request_params), 1);
    rox_check_map_contains(ctx->get_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");

    ck_assert_str_eq(ctx->post_request_uri, "https://x-api.rollout.io/device/get_configuration/123/buid");
    ck_assert_int_eq(hashtable_size(ctx->post_request_params), 5);
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_APP_KEY.name, "123");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_API_VERSION.name, "4.0.0");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_DISTINCT_ID.name, "id");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_BUID.name, "buid");
    rox_check_map_contains(ctx->post_request_params, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name, "123/buid");

    ck_assert_str_eq(cJSON_GetObjectItem(result->parsed_data, "harto")->valuestring, "a");
    ck_assert_int_eq(CONFIGURATION_SOURCE_API, result->source);
    ck_assert_int_eq(0, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(1, ctx->times_post_request_sent);

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

    ctx->status_to_return_to_get_request = 404;
    ctx->status_to_return_to_post_request = 404;

    ConfigurationFetcher *fetcher = _test_create_conf_fetcher(ctx);
    ConfigurationFetchResult *result = configuration_fetcher_fetch(fetcher);
    ck_assert_ptr_null(result);
    ck_assert_int_eq(1, ctx->times_invoker_called);
    ck_assert_int_eq(1, ctx->times_get_request_sent);
    ck_assert_int_eq(1, ctx->times_post_request_sent);

    configuration_fetcher_free(fetcher);
    _request_test_context_free(ctx);
}

END_TEST

//
// ConfigurationFetcherRoxyTests
//

START_TEST (test_will_return_data_when_successful) {
    RequestTestContext *ctx = _request_test_context_create(NULL, NULL);
    ctx->status_to_return_to_get_request = 200;
    ctx->data_to_return_to_get_request = "{\"a\": \"harti\"}";
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
    ctx->status_to_return_to_get_request = 0;
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
    ctx->status_to_return_to_get_request = 404;
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
