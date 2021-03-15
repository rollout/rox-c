#include <assert.h>
#include <check.h>
#include "roxtests.h"
#include "fixtures.h"
#include "core.h"
#include "util.h"

//
// CoreTests
//

typedef struct CoreTestContext {
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    RoxOptions *rox_options;
    RoxCore *core;
    LoggingTestFixture *logging;
    RequestTestFixture *request;
} CoreTestContext;

static CoreTestContext *core_test_context_create(const char *api_key, const char *roxy_url) {
    CoreTestContext *ctx = calloc(1, sizeof(CoreTestContext));
    ctx->sdk_settings = calloc(1, sizeof(SdkSettings));
    ctx->sdk_settings->api_key = api_key ? mem_copy_str(api_key) : NULL;
    ctx->sdk_settings->dev_mode_secret = mem_copy_str("test");
    ctx->rox_options = rox_options_create();
    if (roxy_url) {
        rox_options_set_roxy_url(ctx->rox_options, roxy_url);
    }
    ctx->device_properties = device_properties_create_from_map(ctx->sdk_settings, ctx->rox_options, ROX_EMPTY_MAP);
    ctx->logging = logging_test_fixture_create(RoxLogLevelDebug);
    ctx->request = request_test_fixture_create();
    ctx->request->status_to_return_to_get = 200;
    ctx->request->status_to_return_to_post = 200;
    ctx->request->status_to_return_to_post_json = 200;
    ctx->core = rox_core_create(&ctx->request->config);
    return ctx;
}

static void core_test_context_free(CoreTestContext *ctx) {
    assert(ctx);
    rox_core_free(ctx->core);
    if (ctx->sdk_settings->api_key) {
        free(ctx->sdk_settings->api_key);
    }
    if (ctx->sdk_settings->dev_mode_secret) {
        free(ctx->sdk_settings->dev_mode_secret);
    }
    free(ctx->sdk_settings);
    device_properties_free(ctx->device_properties);
    rox_options_free(ctx->rox_options);
    logging_test_fixture_free(ctx->logging);
    request_test_fixture_free(ctx->request);
    free(ctx);
}

START_TEST (test_will_check_null_api_key) {
    CoreTestContext *ctx = core_test_context_create(NULL, NULL);
    RoxStateCode status = rox_core_setup(ctx->core, ctx->sdk_settings, ctx->device_properties, ctx->rox_options);
    ck_assert_int_eq(RoxErrorEmptyApiKey, status);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Invalid rollout apikey");
    core_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_check_empty_api_key) {
    CoreTestContext *ctx = core_test_context_create("", NULL);
    RoxStateCode status = rox_core_setup(ctx->core, ctx->sdk_settings, ctx->device_properties, ctx->rox_options);
    ck_assert_int_eq(RoxErrorEmptyApiKey, status);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Invalid rollout apikey");
    core_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_check_invalid_api_key) {
    CoreTestContext *ctx = core_test_context_create("aaaaaaaaaaaaaaaaaaaaaaag", NULL);
    RoxStateCode status = rox_core_setup(ctx->core, ctx->sdk_settings, ctx->device_properties, ctx->rox_options);
    ck_assert_int_eq(RoxErrorInvalidApiKey, status);
    logging_test_fixture_check_log_message(ctx->logging, RoxLogLevelError, "Illegal rollout apikey");
    core_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_check_core_setup_when_options_with_roxy) {
    CoreTestContext *ctx = core_test_context_create("doesn't matter", "http://localhost");
    RoxStateCode status = rox_core_setup(ctx->core, ctx->sdk_settings, ctx->device_properties, ctx->rox_options);
    ck_assert_int_eq(RoxInitialized, status);
    core_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_check_core_setup_when_no_options) {
    CoreTestContext *ctx = core_test_context_create("aaaaaaaaaaaaaaaaaaaaaaaa", NULL);
    RoxStateCode status = rox_core_setup(ctx->core, ctx->sdk_settings, ctx->device_properties, NULL);
    ck_assert_int_eq(RoxInitialized, status);
    core_test_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_will_check_null_api_key),
        ROX_TEST_CASE(test_will_check_empty_api_key),
        ROX_TEST_CASE(test_will_check_invalid_api_key),
        ROX_TEST_CASE(test_will_check_core_setup_when_options_with_roxy),
        ROX_TEST_CASE(test_will_check_core_setup_when_no_options)
)
