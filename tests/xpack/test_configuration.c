#include <check.h>
#include <assert.h>
#include <roxx/extensions.h>
#include "roxtests.h"
#include "xpack/configuration.h"

//
// ConfigurationFetchedInvokerTests
//

typedef struct ROX_INTERNAL ConfigurationFetchedInvokerTestContext {
    ConfigurationFetchedInvoker *invoker;
    ExperimentRepository *experiment_repository;
    Parser *parser;
    InternalFlags *flags;
    SdkSettings *sdk_settings;
    XConfigurationFetchedInvoker *x_invoker;
    int times_invoked;
    int times_fetch_invoked;
    List *args;
} ConfigurationFetchedInvokerTestContext;

static void _check_conf_fetched_args(
        ConfigurationFetchedInvokerTestContext *ctx,
        FetchStatus status,
        const char *date,
        bool has_changes,
        FetcherError error) {

    assert(ctx);

    ck_assert_int_eq(1, list_size(ctx->args));

    ConfigurationFetchedArgs *args;
    ck_assert_int_eq(list_get_first(ctx->args, (void **) &args), CC_OK);
    ck_assert_int_eq(status, args->fetcher_status);
    ck_assert_int_eq(has_changes, args->has_changes);
    ck_assert_int_eq(error, args->error_details);

    if (date) {
        ck_assert_str_eq(date, args->creation_date);
    } else {
        ck_assert_ptr_null(args->creation_date);
    }
}

static void _test_configuration_fetch_func(void *target) {
    assert(target);
    ConfigurationFetchedInvokerTestContext *ctx = (ConfigurationFetchedInvokerTestContext *) target;
    ++ctx->times_fetch_invoked;
}

static void _test_configuration_fetched_handler(void *target, ConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    ConfigurationFetchedInvokerTestContext *ctx = (ConfigurationFetchedInvokerTestContext *) target;
    list_add(ctx->args, configuration_fetched_args_copy(args));
}

static ConfigurationFetchedInvokerTestContext *_configuration_fetched_invoker_test_context_create() {
    ConfigurationFetchedInvokerTestContext *ctx = calloc(1, sizeof(ConfigurationFetchedInvokerTestContext));
    ctx->invoker = configuration_fetched_invoker_create();
    ctx->experiment_repository = experiment_repository_create();
    ctx->parser = parser_create();
    ctx->flags = internal_flags_create(ctx->experiment_repository, ctx->parser);
    ctx->sdk_settings = sdk_settings_create("test", "test");
    ctx->x_invoker = x_configuration_fetched_invoker_create(
            ctx->flags, ctx->sdk_settings, ctx, &_test_configuration_fetch_func);
    list_new(&ctx->args);
    configuration_fetched_invoker_register_handler(ctx->invoker, ctx->x_invoker, &x_configuration_fetched_handler);
    configuration_fetched_invoker_register_handler(ctx->invoker, ctx, &_test_configuration_fetched_handler);
    return ctx;
}

static void _configuration_fetched_invoker_test_context_free(ConfigurationFetchedInvokerTestContext *ctx) {
    assert(ctx);
    x_configuration_fetched_invoker_free(ctx->x_invoker);
    internal_flags_free(ctx->flags);
    sdk_settings_free(ctx->sdk_settings);
    experiment_repository_free(ctx->experiment_repository);
    parser_free(ctx->parser);
    configuration_fetched_invoker_free(ctx->invoker);
    list_destroy_cb(ctx->args, (void (*)(void *)) &configuration_fetched_args_free);
    free(ctx);
}

START_TEST (test_configuration_invoker_with_no_subscriber_no_exception) {
    ConfigurationFetchedInvokerTestContext *ctx = _configuration_fetched_invoker_test_context_create();
    configuration_fetched_invoker_invoke(ctx->invoker, AppliedFromEmbedded, "2012-04-23T18:25:43.511Z", true);
    ck_assert_int_eq(0, ctx->times_fetch_invoked);
    _configuration_fetched_invoker_test_context_free(ctx);
}

END_TEST

START_TEST (test_configuration_fetched_args_constructor) {
    FetchStatus status = AppliedFromEmbedded;
    const char *time = "2012-04-23T18:25:43.511Z";
    bool has_changes = true;

    ConfigurationFetchedArgs *args = configuration_fetched_args_create(status, time, has_changes);

    ck_assert_int_eq(status, args->fetcher_status);
    ck_assert_str_eq(time, args->creation_date);
    ck_assert_int_eq(has_changes, args->has_changes);
    ck_assert_int_eq(NoError, args->error_details);

    FetcherError error = SignatureVerificationError;
    ConfigurationFetchedArgs *args2 = configuration_fetched_args_create_error(error);

    ck_assert_int_eq(ErrorFetchedFailed, args2->fetcher_status);
    ck_assert_ptr_null(args2->creation_date);
    ck_assert_int_eq(false, args2->has_changes);
    ck_assert_int_eq(SignatureVerificationError, args2->error_details);
}

END_TEST

START_TEST (test_configuration_invoker_invoke_with_error) {
    ConfigurationFetchedInvokerTestContext *ctx = _configuration_fetched_invoker_test_context_create();
    configuration_fetched_invoker_invoke_error(ctx->invoker, UnknownError);
    ck_assert_int_eq(0, ctx->times_fetch_invoked);
    _check_conf_fetched_args(ctx, ErrorFetchedFailed, NULL, false, UnknownError);
    _configuration_fetched_invoker_test_context_free(ctx);
}

END_TEST

START_TEST (test_configuration_invoker_invoke_ok) {
    FetchStatus status = AppliedFromEmbedded;
    const char *time = "2012-04-23T18:25:43.511Z";
    bool has_changes = true;
    ConfigurationFetchedInvokerTestContext *ctx = _configuration_fetched_invoker_test_context_create();
    configuration_fetched_invoker_invoke(ctx->invoker, status, time, has_changes);
    ck_assert_int_eq(0, ctx->times_fetch_invoked);
    _check_conf_fetched_args(ctx, status, time, has_changes, NoError);
    _configuration_fetched_invoker_test_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
// ConfigurationFetchedInvokerTests
        ROX_TEST_CASE(test_configuration_invoker_with_no_subscriber_no_exception),
        ROX_TEST_CASE(test_configuration_fetched_args_constructor),
        ROX_TEST_CASE(test_configuration_invoker_invoke_with_error),
        ROX_TEST_CASE(test_configuration_invoker_invoke_ok)
)
