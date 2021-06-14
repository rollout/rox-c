#include <stdlib.h>
#include <assert.h>
#include <core/configuration/models.h>
#include "core/configuration.h"
#include "xpack/security.h"
#include "roxtests.h"

bool _test_true_signature_verifier(void *target, SignatureVerifier *verifier, const char *data,
                                   const char *signature_base64) {
    assert(verifier);
    assert(data);
    assert(signature_base64);
    return true;
}

bool _test_false_signature_verifier(void *target, SignatureVerifier *verifier, const char *data,
                                    const char *signature_base64) {
    assert(verifier);
    assert(data);
    assert(signature_base64);
    return false;
}

bool _test_true_api_key_verifier(APIKeyVerifier *verifier, const char *api_key) {
    assert(verifier);
    assert(api_key);
    return true;
}

bool _test_false_api_key_verifier(APIKeyVerifier *verifier, const char *api_key) {
    assert(verifier);
    assert(api_key);
    return false;
}

void _test_error_report(ErrorReporter *reporter, const char *fmt, ...) {
    assert(reporter);
    assert(fmt);
    // Stub
}

typedef struct ConfigurationTestContext {
    SignatureVerifier *signature_verifier;
    ErrorReporter *error_reporter;
    SdkSettings *sdk_settings;
    APIKeyVerifier *key_verifier;
    ConfigurationFetchedInvoker *invoker;
    ConfigurationFetchResult *result;
    ConfigurationParser *parser;
    bool configuration_fetched;
    RoxFetcherError fetcher_error;
} ConfigurationTestContext;

static void _test_configuration_fetched_handler(void *target, RoxConfigurationFetchedArgs *args) {
    assert(target);
    ConfigurationTestContext *test_context = (ConfigurationTestContext *) target;
    test_context->configuration_fetched = true;
    test_context->fetcher_error = args->error_details;
}

static ConfigurationTestContext *configuration_test_context_create(
        const char *json_str,
        ConfigurationSource source,
        bool signature_verified,
        bool api_key_verified) {

    cJSON *json = cJSON_Parse(json_str);
    assert(json);

    ConfigurationTestContext *context = calloc(1, sizeof(ConfigurationTestContext));
    context->sdk_settings = sdk_settings_create("12345", "12345");
    SignatureVerifierConfig signature_verifier_config = {
            context,
            signature_verified
            ? &_test_true_signature_verifier
            : &_test_false_signature_verifier
    };

    APIKeyVerifierConfig config = {
            context->sdk_settings,
            api_key_verified
            ? &_test_true_api_key_verifier
            : &_test_false_api_key_verifier
    };

    ErrorReporterConfig error_reporter_config = {&_test_error_report};

    context->signature_verifier = signature_verifier_create(&signature_verifier_config);
    context->error_reporter = error_reporter_create(&error_reporter_config);
    context->result = configuration_fetch_result_create(json, source);
    context->key_verifier = api_key_verifier_create(&config);
    context->invoker = configuration_fetched_invoker_create();
    configuration_fetched_invoker_register_handler(context->invoker, context, &_test_configuration_fetched_handler);
    context->parser = configuration_parser_create(
            context->signature_verifier,
            context->error_reporter,
            context->key_verifier,
            context->invoker);

    return context;
}

static Configuration *configuration_test_context_parse(ConfigurationTestContext *context) {
    assert(context);
    return configuration_parser_parse(
            context->parser,
            context->result);
}

static void configuration_test_context_free(ConfigurationTestContext *context) {
    assert(context);
    error_reporter_free(context->error_reporter);
    api_key_verifier_free(context->key_verifier);
    signature_verifier_free(context->signature_verifier);
    configuration_fetched_invoker_free(context->invoker);
    configuration_fetch_result_free(context->result);
    configuration_parser_free(context->parser);
    sdk_settings_free(context->sdk_settings);
    free(context);
}

START_TEST (test_will_return_null_when_unexpected_exception) {
    const char *json_str = "{\n"
                           "   \"nodata\":\"{\\\"application\\\":\\\"12345\\\",\\\"targetGroups\\\":[{\\\"condition\\\":\\\"eq(true,true)\\\",\\\"_id\\\":\\\"12345\\\"},{\\\"_id\\\":\\\"123456\\\",\\\"condition\\\":\\\"eq(true,true)\\\"}],\\\"experiments\\\":[{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"FeatureFlags.isFeatureFlagsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Feature Flags Drawer Item\\\",\\\"_id\\\":\\\"1\\\"},{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"Invitations.isInvitationsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Enable Modern Invitations\\\",\\\"_id\\\":\\\"2\\\"}]}\",\n"
                           "   \"signature_v0\":\"K/bEQCkRXa6+uFr5H2jCRCaVgmtsTwbgfrFGVJ9NebfMH8CgOhCDIvF4TM1Vyyl0bGS9a4r4Qgi/g63NDBWk0ZbRrKAUkVG56V3/bI2GDHxFvRNrNbiPmFv/wmLLuwgh1mdzU0EwLG4M7yXoNXtMr6Jli8t4xfBOaWW1g0QpASkiWa7kdTamVip/1QygyUuhX5hOyUMpy4Ny9Hi/QPvVBn6GDMxQtxpLfTavU9cBly2D7Ex8Z7sUUOKeoEJcdsoF1QzH14XvA2HQSICESz7D/uld0PNdG0tMj9NlAZfki8eY2KuUe/53Z0Og5WrqQUxiAdPuJoZr6+kSqlASZrrkYw==\",\n"
                           "   \"signed_date\":\"2018-01-09T19:02:00.720Z\"\n"
                           "}";

    ConfigurationTestContext *context = configuration_test_context_create(json_str, CONFIGURATION_SOURCE_API, true,
                                                                          true);
    Configuration *config = configuration_test_context_parse(context);
    ck_assert_ptr_null(config);
    ck_assert(context->configuration_fetched);
    ck_assert_int_eq(context->fetcher_error, UnknownError);
    configuration_test_context_free(context);
}

END_TEST

START_TEST (test_will_return_null_when_wrong_signature) {
    const char *json_str = "{\n"
                           "   \"data\":\"{\\\"application\\\":\\\"12345\\\",\\\"targetGroups\\\":[{\\\"condition\\\":\\\"eq(true,true)\\\",\\\"_id\\\":\\\"12345\\\"},{\\\"_id\\\":\\\"123456\\\",\\\"condition\\\":\\\"eq(true,true)\\\"}],\\\"experiments\\\":[{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"FeatureFlags.isFeatureFlagsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Feature Flags Drawer Item\\\",\\\"_id\\\":\\\"1\\\"},{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"Invitations.isInvitationsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Enable Modern Invitations\\\",\\\"_id\\\":\\\"2\\\"}] } \",\n"
                           "   \"signature_v0\":\"wrongK/bEQCkRXa6+uFr5H2jCRCaVgmtsTwbgfrFGVJ9NebfMH8CgOhCDIvF4TM1Vyyl0bGS9a4r4Qgi/g63NDBWk0ZbRrKAUkVG56V3/bI2GDHxFvRNrNbiPmFv/wmLLuwgh1mdzU0EwLG4M7yXoNXtMr6Jli8t4xfBOaWW1g0QpASkiWa7kdTamVip/1QygyUuhX5hOyUMpy4Ny9Hi/QPvVBn6GDMxQtxpLfTavU9cBly2D7Ex8Z7sUUOKeoEJcdsoF1QzH14XvA2HQSICESz7D/uld0PNdG0tMj9NlAZfki8eY2KuUe/53Z0Og5WrqQUxiAdPuJoZr6+kSqlASZrrkYw==\",\n"
                           "   \"signed_date\":\"2018-01-09T19:02:00.720Z\"\n"
                           "}";

    ConfigurationTestContext *context = configuration_test_context_create(json_str, CONFIGURATION_SOURCE_API, false,
                                                                          true);
    Configuration *config = configuration_test_context_parse(context);
    ck_assert_ptr_null(config);
    ck_assert(context->configuration_fetched);
    ck_assert_int_eq(context->fetcher_error, SignatureVerificationError);
    configuration_test_context_free(context);
}

END_TEST

START_TEST (test_will_return_null_when_wrong_api_key) {
    const char *json_str = "{\n"
                           "   \"data\":\"{\\\"application\\\":\\\"12345\\\",\\\"targetGroups\\\":[{\\\"condition\\\":\\\"eq(true,true)\\\",\\\"_id\\\":\\\"12345\\\"},{\\\"_id\\\":\\\"123456\\\",\\\"condition\\\":\\\"eq(true,true)\\\"}],\\\"experiments\\\":[{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"FeatureFlags.isFeatureFlagsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Feature Flags Drawer Item\\\",\\\"_id\\\":\\\"1\\\"},{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"Invitations.isInvitationsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Enable Modern Invitations\\\",\\\"_id\\\":\\\"2\\\"}] }\",\n"
                           "   \"signature_v0\":\"K/bEQCkRXa6+uFr5H2jCRCaVgmtsTwbgfrFGVJ9NebfMH8CgOhCDIvF4TM1Vyyl0bGS9a4r4Qgi/g63NDBWk0ZbRrKAUkVG56V3/bI2GDHxFvRNrNbiPmFv/wmLLuwgh1mdzU0EwLG4M7yXoNXtMr6Jli8t4xfBOaWW1g0QpASkiWa7kdTamVip/1QygyUuhX5hOyUMpy4Ny9Hi/QPvVBn6GDMxQtxpLfTavU9cBly2D7Ex8Z7sUUOKeoEJcdsoF1QzH14XvA2HQSICESz7D/uld0PNdG0tMj9NlAZfki8eY2KuUe/53Z0Og5WrqQUxiAdPuJoZr6+kSqlASZrrkYw==\",\n"
                           "   \"signed_date\":\"2018-01-09T19:02:00.720Z\"\n"
                           "}";

    ConfigurationTestContext *context = configuration_test_context_create(json_str, CONFIGURATION_SOURCE_API, true,
                                                                          false);
    Configuration *config = configuration_test_context_parse(context);
    ck_assert_ptr_null(config);
    ck_assert(context->configuration_fetched);
    ck_assert_int_eq(context->fetcher_error, MismatchAppKey);
    configuration_test_context_free(context);
}

END_TEST

START_TEST (test_will_parse_experiments_and_target_groups) {
    const char *json_str = "{\n"
                           "   \"data\":\"{\\\"application\\\":\\\"12345\\\",\\\"targetGroups\\\":[{\\\"condition\\\":\\\"eq(true,true)\\\",\\\"_id\\\":\\\"12345\\\"},{\\\"_id\\\":\\\"123456\\\",\\\"condition\\\":\\\"eq(true,true)\\\"}],\\\"experiments\\\":[{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"FeatureFlags.isFeatureFlagsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Feature Flags Drawer Item\\\",\\\"_id\\\":\\\"1\\\",\\\"labels\\\":[\\\"label1\\\"]},{\\\"deploymentConfiguration\\\":{\\\"condition\\\":\\\"ifThen(and(true, true)\\\"},\\\"featureFlags\\\":[{\\\"name\\\":\\\"Invitations.isInvitationsEnabled\\\"}],\\\"archived\\\":false,\\\"name\\\":\\\"Enable Modern Invitations\\\",\\\"_id\\\":\\\"2\\\"}] }\",\n"
                           "   \"signature_v0\":\"K/bEQCkRXa6+uFr5H2jCRCaVgmtsTwbgfrFGVJ9NebfMH8CgOhCDIvF4TM1Vyyl0bGS9a4r4Qgi/g63NDBWk0ZbRrKAUkVG56V3/bI2GDHxFvRNrNbiPmFv/wmLLuwgh1mdzU0EwLG4M7yXoNXtMr6Jli8t4xfBOaWW1g0QpASkiWa7kdTamVip/1QygyUuhX5hOyUMpy4Ny9Hi/QPvVBn6GDMxQtxpLfTavU9cBly2D7Ex8Z7sUUOKeoEJcdsoF1QzH14XvA2HQSICESz7D/uld0PNdG0tMj9NlAZfki8eY2KuUe/53Z0Og5WrqQUxiAdPuJoZr6+kSqlASZrrkYw==\",\n"
                           "   \"signed_date\":\"2018-01-09T19:02:00.720Z\"\n"
                           "}";

    ConfigurationTestContext *context = configuration_test_context_create(json_str, CONFIGURATION_SOURCE_API, true,
                                                                          true);
    Configuration *conf = configuration_test_context_parse(context);
    ck_assert_ptr_nonnull(conf);
    ck_assert(!context->configuration_fetched);
    ck_assert(!context->fetcher_error);

    ck_assert_int_eq(rox_list_size(conf->target_groups), 2);

    TargetGroupModel *target_group_model;
    ck_assert(rox_list_get_at(conf->target_groups, 0, (void **) &target_group_model));
    ck_assert_str_eq(target_group_model->id, "12345");
    ck_assert_str_eq(target_group_model->condition, "eq(true,true)");

    ck_assert(rox_list_get_at(conf->target_groups, 1, (void **) &target_group_model));
    ck_assert_str_eq(target_group_model->id, "123456");
    ck_assert_str_eq(target_group_model->condition, "eq(true,true)");

    ck_assert_int_eq(rox_list_size(conf->experiments), 2);

    ExperimentModel *experiment_model;
    ck_assert(rox_list_get_at(conf->experiments, 0, (void **) &experiment_model));
    ck_assert_str_eq(experiment_model->id, "1");
    ck_assert_str_eq(experiment_model->name, "Feature Flags Drawer Item");
    ck_assert_str_eq(experiment_model->condition, "ifThen(and(true, true)");
    ck_assert_int_eq(experiment_model->archived, false);
    ck_assert_int_eq(rox_list_size(experiment_model->flags), 1);

    char *flag;
    ck_assert(rox_list_get_first(experiment_model->flags, (void **) &flag));
    ck_assert_str_eq(flag, "FeatureFlags.isFeatureFlagsEnabled");

    ck_assert_int_eq(rox_set_size(experiment_model->labels), 1);

    char *label;
    RoxSetIter *iter = rox_set_iter_create();
    rox_set_iter_init(iter, experiment_model->labels);
    ck_assert(rox_set_iter_next(iter, (void **) &label));
    ck_assert_str_eq(label, "label1");
    rox_set_iter_free(iter);

    ck_assert(rox_list_get_at(conf->experiments, 1, (void **) &experiment_model));
    ck_assert_str_eq(experiment_model->id, "2");
    ck_assert_str_eq(experiment_model->name, "Enable Modern Invitations");
    ck_assert_str_eq(experiment_model->condition, "ifThen(and(true, true)");
    ck_assert_int_eq(experiment_model->archived, false);

    ck_assert_int_eq(rox_list_size(experiment_model->flags), 1);
    ck_assert(rox_list_get_first(experiment_model->flags, (void **) &flag));
    ck_assert_str_eq(flag, "Invitations.isInvitationsEnabled");

    ck_assert_int_eq(rox_set_size(experiment_model->labels), 0);

    configuration_test_context_free(context);
    configuration_free(conf);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_will_return_null_when_unexpected_exception),
        ROX_TEST_CASE(test_will_return_null_when_wrong_signature),
        ROX_TEST_CASE(test_will_return_null_when_wrong_api_key),
        ROX_TEST_CASE(test_will_parse_experiments_and_target_groups)
)
