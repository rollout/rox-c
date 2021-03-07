#include <stdbool.h>
#include <core/consts.h>
#include <assert.h>
#include "roxtests.h"
#include "eval/parser.h"
#include "core/repositories.h"
#include "core/client.h"
#include "core/entities.h"
#include "collections.h"

//
// DynamicApiTests
//

typedef struct ClientTestContext {
    char *last_impression_value;
    int impressions;
    ImpressionInvoker *impression_invoker;
    Parser *parser;
    FlagRepository *flag_repo;
    ExperimentRepository *exp_repo;
    FlagSetter *flag_setter;
    EntitiesProvider *entities_provider;
    RoxDynamicApi *api;
} ClientTestContext;

static void test_impression_handler_func(
        void *target,
        RoxReportingValue *value,
        RoxContext *context) {
    ClientTestContext *handler = (ClientTestContext *) target;
    if (handler->last_impression_value) {
        free(handler->last_impression_value);
    }
    handler->last_impression_value = value->value ? mem_copy_str(value->value) : NULL;
    ++handler->impressions;
}

static ClientTestContext *client_test_context_create() {
    ClientTestContext *ctx = calloc(1, sizeof(ClientTestContext));
    ctx->impression_invoker = impression_invoker_create();
    impression_invoker_register(ctx->impression_invoker, ctx, &test_impression_handler_func);
    ctx->parser = parser_create();
    ctx->flag_repo = flag_repository_create();
    ctx->exp_repo = experiment_repository_create();
    ctx->flag_setter = flag_setter_create(ctx->flag_repo, ctx->parser, ctx->exp_repo, ctx->impression_invoker);
    ctx->entities_provider = entities_provider_create();
    ctx->api = dynamic_api_create(ctx->flag_repo, ctx->entities_provider);
    return ctx;
}

static void client_test_context_set_experiment(
        ClientTestContext *ctx,
        const char *flag_name,
        const char *condition) {
    experiment_repository_set_experiments(ctx->exp_repo, ROX_LIST(
            experiment_model_create("1", flag_name, condition, false,
                                    ROX_LIST_COPY_STR(flag_name), ROX_EMPTY_SET, "stam")
    ));
    flag_setter_set_experiments(ctx->flag_setter);
}

static void client_test_context_free(ClientTestContext *ctx) {
    assert(ctx);
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
    }
    rox_dynamic_api_free(ctx->api);
    entities_provider_free(ctx->entities_provider);
    flag_setter_free(ctx->flag_setter);
    flag_repository_free(ctx->flag_repo);
    parser_free(ctx->parser);
    experiment_repository_free(ctx->exp_repo);
    impression_invoker_free(ctx->impression_invoker);
    free(ctx);
}

START_TEST (test_is_enabled) {

    ClientTestContext *ctx = client_test_context_create();

    ck_assert(rox_dynamic_api_is_enabled(ctx->api, "default.newFlag", true));
    ck_assert(variant_is_flag(flag_repository_get_flag(ctx->flag_repo, "default.newFlag")));
    ck_assert_str_eq(ctx->last_impression_value, "true");
    ck_assert_int_eq(ctx->impressions, 1);

    ck_assert(variant_get_bool(flag_repository_get_flag(ctx->flag_repo, "default.newFlag"), NULL, NULL));
    ck_assert_str_eq(ctx->last_impression_value, "true");
    ck_assert_int_eq(ctx->impressions, 2);

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newFlag", false));
    ck_assert_str_eq(ctx->last_impression_value, "false");
    ck_assert_int_eq(ctx->impressions, 3);

    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));

    client_test_context_set_experiment(ctx, "default.newFlag", "and(true, true)");

    ck_assert(rox_dynamic_api_is_enabled(ctx->api, "default.newFlag", false));
    ck_assert_str_eq(ctx->last_impression_value, "true");
    ck_assert_int_eq(ctx->impressions, 4);

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_is_enabled_after_setup) {
    ClientTestContext *ctx = client_test_context_create();
    client_test_context_set_experiment(ctx, "default.newFlag", "and(true, true)");

    ck_assert(rox_dynamic_api_is_enabled(ctx->api, "default.newFlag", false));

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_is_enabled_different_type_call) {

    ClientTestContext *ctx = client_test_context_create();

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newVariant", false));
    ck_assert_str_eq(ctx->last_impression_value, "false");
    ck_assert_int_eq(ctx->impressions, 1);

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"true\", \"true\")");

    ck_assert_double_eq(3.4, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 3.4));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "3.4");

    rox_check_and_free(rox_dynamic_api_get_string(ctx->api, "default.newVariant", "1"), "true");
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "true");

    ck_assert_int_eq(2, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 2));
    ck_assert_int_eq(ctx->impressions, 4);
    ck_assert_str_eq(ctx->last_impression_value, "2");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_is_enabled_wrong_experiment_type) {

    ClientTestContext *ctx = client_test_context_create();
    client_test_context_set_experiment(ctx, "default.newFlag", "\"otherValue\"");

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newFlag", false));
    ck_assert_str_eq(ctx->last_impression_value, "false");
    ck_assert_int_eq(ctx->impressions, 1);

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_string) {

    ClientTestContext *ctx = client_test_context_create();

    RoxList *options = ROX_LIST_COPY_STR("A", "B", "C");
    rox_check_and_free(rox_dynamic_api_get_string_ctx(ctx->api, "default.newVariant", "A", options, NULL), "A");
    ck_assert_str_eq(ctx->last_impression_value, "A");
    ck_assert_int_eq(ctx->impressions, 1);

    RoxStringBase *flag = flag_repository_get_flag(ctx->flag_repo, "default.newVariant");
    rox_check_and_free(variant_get_string(flag, NULL, NULL), "A");
    ck_assert_str_eq(ctx->last_impression_value, "A");
    ck_assert_int_eq(ctx->impressions, 2);

    rox_check_and_free(rox_dynamic_api_get_string_ctx(ctx->api, "default.newVariant", "B", options, NULL), "B");
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));
    ck_assert_str_eq(ctx->last_impression_value, "B");
    ck_assert_int_eq(ctx->impressions, 3);

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"B\", \"A\")");

    rox_check_and_free(rox_dynamic_api_get_string_ctx(ctx->api, "default.newVariant", "A", options, NULL), "B");
    ck_assert_str_eq(ctx->last_impression_value, "B");
    ck_assert_int_eq(ctx->impressions, 4);

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_string_ignore_variation_null_when_variant_exists) {
    ClientTestContext *ctx = client_test_context_create();

    RoxList *options = ROX_LIST_COPY_STR("A", "B", "C");
    rox_check_and_free(rox_dynamic_api_get_string_ctx(ctx->api, "default.newVariant", "A", options, NULL), "A");
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "A");

    options = ROX_LIST_COPY_STR("A", "B", NULL);
    rox_check_and_free(rox_dynamic_api_get_string_ctx(ctx->api, "default.newVariant", "A", options, NULL), "A");
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "A");
    rox_list_free_cb(options, free);

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_string_different_type_call) {
    ClientTestContext *ctx = client_test_context_create();

    rox_check_and_free(rox_dynamic_api_get_string(ctx->api, "default.newVariant", "value"), "value");
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "value");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"val1\", \"val2\")");

    ck_assert_double_eq(3.4, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 3.4));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "3.4");

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newVariant", false));
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "false");

    ck_assert_int_eq(2, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 2));
    ck_assert_int_eq(ctx->impressions, 4);
    ck_assert_str_eq(ctx->last_impression_value, "2");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"3\", \"4\")");

    ck_assert_int_eq(3, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 1));
    ck_assert_int_eq(ctx->impressions, 5);
    ck_assert_str_eq(ctx->last_impression_value, "3");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_int) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_int_eq(1, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 1));
    ck_assert(variant_is_int(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "1");

    ck_assert_int_eq(2, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 2));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "2");
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"3\", \"4\")");
    ck_assert_int_eq(3, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 2));
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "3");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_int_different_type_call) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_int_eq(1, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 1));
    ck_assert(variant_is_int(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "1");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"2\", \"3\")");

    ck_assert_double_eq(2, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 3.4));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "2");

    rox_check_and_free(rox_dynamic_api_get_string(ctx->api, "default.newVariant", "1"), "2");
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "2");

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newVariant", true));
    ck_assert_int_eq(ctx->impressions, 4);
    ck_assert_str_eq(ctx->last_impression_value, "false");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_int_wrong_experiment_type) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_int_eq(1, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 1));
    ck_assert(variant_is_int(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "1");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"3.5\", \"4.1\")");
    ck_assert_int_eq(2, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 2));
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "2");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_double) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_double_eq(1.1, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 1.1));
    ck_assert(variant_is_double(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "1.1");

    ck_assert_double_eq(2.2, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 2.2));
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "2.2");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"3.3\", \"4.4\")");
    ck_assert_double_eq(3.3, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 2.2));
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "3.3");

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_double_different_type_call) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_double_eq(2.1, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 2.1));
    ck_assert(variant_is_double(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "2.1");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"3.3\", \"4.4\")");

    ck_assert_int_eq(1, rox_dynamic_api_get_int(ctx->api, "default.newVariant", 1));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "1");

    rox_check_and_free(rox_dynamic_api_get_string(ctx->api, "default.newVariant", "1"), "3.3");
    ck_assert_int_eq(ctx->impressions, 3);
    ck_assert_str_eq(ctx->last_impression_value, "3.3");

    ck_assert(!rox_dynamic_api_is_enabled(ctx->api, "default.newVariant", false));
    ck_assert_int_eq(ctx->impressions, 4);
    ck_assert_str_eq(ctx->last_impression_value, "false");

    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(ctx->flag_repo)));

    client_test_context_free(ctx);
}

END_TEST

START_TEST (test_get_double_wrong_experiment_type) {
    ClientTestContext *ctx = client_test_context_create();

    ck_assert_double_eq(1.1, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 1.1));
    ck_assert(variant_is_double(flag_repository_get_flag(ctx->flag_repo, "default.newVariant")));
    ck_assert_int_eq(ctx->impressions, 1);
    ck_assert_str_eq(ctx->last_impression_value, "1.1");

    client_test_context_set_experiment(ctx, "default.newVariant", "ifThen(true, \"aaa\", \"bbb\")");
    ck_assert_double_eq(2.2, rox_dynamic_api_get_double(ctx->api, "default.newVariant", 2.2));
    ck_assert_int_eq(ctx->impressions, 2);
    ck_assert_str_eq(ctx->last_impression_value, "2.2");

    client_test_context_free(ctx);
}

END_TEST

//
// InternalFlagsTests
//

START_TEST (test_will_return_false_when_no_experiment) {
    Parser *parser = parser_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    InternalFlags *internal_flags = internal_flags_create(exp_repo, parser);
    ck_assert(!internal_flags_is_enabled(internal_flags, "stam"));
    internal_flags_free(internal_flags);
    experiment_repository_free(exp_repo);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_return_false_when_expression_is_null) {
    Parser *parser = parser_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id", "name", "stam", false, ROX_LIST_COPY_STR("stam"), NULL, "stam")
    ));
    InternalFlags *internal_flags = internal_flags_create(exp_repo, parser);
    ck_assert(!internal_flags_is_enabled(internal_flags, "stam"));
    internal_flags_free(internal_flags);
    experiment_repository_free(exp_repo);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_return_false_when_expression_is_false) {
    Parser *parser = parser_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id", "name", "false", false, ROX_LIST_COPY_STR("stam"), NULL, "stam")
    ));
    InternalFlags *internal_flags = internal_flags_create(exp_repo, parser);
    ck_assert(!internal_flags_is_enabled(internal_flags, "stam"));
    internal_flags_free(internal_flags);
    experiment_repository_free(exp_repo);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_return_true_when_expression_is_true) {
    Parser *parser = parser_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id", "name", "true", false, ROX_LIST_COPY_STR("stam"), NULL, "stam")
    ));
    InternalFlags *internal_flags = internal_flags_create(exp_repo, parser);
    ck_assert(internal_flags_is_enabled(internal_flags, "stam"));
    internal_flags_free(internal_flags);
    experiment_repository_free(exp_repo);
    parser_free(parser);
}

END_TEST

//
// MD5GeneratorTests
//

START_TEST (test_will_check_md5_uses_right_props) {
    RoxMap *props = rox_map_create();
    rox_map_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "plat");
    RoxList *generators = ROX_LIST(&ROX_PROPERTY_TYPE_PLATFORM);
    char *md5 = md5_generator_generate(props, generators, NULL);
    rox_check_and_free(md5, "1380AFEBC7CE22DE7B3450F8CAB86D2C");
    rox_map_free(props);
    rox_list_free(generators);
}

END_TEST

START_TEST (test_will_check_md5_not_using_all_props) {
    RoxMap *props = rox_map_create();
    rox_map_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "dev");
    rox_map_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "plat");
    RoxList *generators = ROX_LIST(&ROX_PROPERTY_TYPE_PLATFORM);
    char *md5 = md5_generator_generate(props, generators, NULL);
    rox_check_and_free(md5, "1380AFEBC7CE22DE7B3450F8CAB86D2C");
    rox_map_free(props);
    rox_list_free(generators);
}

END_TEST

START_TEST (test_will_check_md5_with_objects) {
    RoxMap *props = rox_map_create();
    rox_map_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "22");
    rox_map_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "True"); // Dotnet legacy, true becomes True
    RoxList *generators = ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET);
    char *md5 = md5_generator_generate(props, generators, NULL);
    rox_check_and_free(md5, "D3816631EDE04D536EAEB479FE5829FD");
    rox_map_free(props);
    rox_list_free(generators);
}

END_TEST

START_TEST (test_will_check_md5_with_json_object) {
    RoxMap *props = rox_map_create();
    rox_map_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "[{\"key\":\"value\"}]");
    rox_map_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "value"); // Dotnet legacy, true becomes True
    RoxList *generators = ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET);
    char *md5 = md5_generator_generate(props, generators, NULL);
    rox_check_and_free(md5, "AA16F2AA33D095940A93C991B00D55C7");
    rox_map_free(props);
    rox_list_free(generators);
}

END_TEST

//
// BUIDTests
//

START_TEST (test_will_generate_correct_md5_value) {
    SdkSettings sdk_settings = {"test", "test"};
    RoxOptions *options = rox_options_create();
    DeviceProperties *device_props = device_properties_create_from_map(&sdk_settings, options, ROX_MAP(
            "app_key", ROX_COPY("123"),
            "api_version", ROX_COPY("4.0.0"),
            "platform", ROX_COPY("plat"),
            "lib_version", ROX_COPY("1.5.0")
    ));

    BUID *buid = buid_create(device_props);
    ck_assert_str_eq(buid_get_value(buid), "234A32BB4341EAFD91FC8D0395F4E66F");
    device_properties_free(device_props);
    buid_free(buid);

    DeviceProperties *device_props2 = device_properties_create_from_map(&sdk_settings, options, ROX_MAP(
            "app_key", ROX_COPY("122"),
            "api_version", ROX_COPY("4.0.0"),
            "platform", ROX_COPY("plat"),
            "lib_version", ROX_COPY("1.5.0")
    ));

    BUID *buid2 = buid_create(device_props2);
    ck_assert_str_eq(buid_get_value(buid2), "F5F30C84B8A806E0004043864724A56E");
    buid_free(buid2);
    device_properties_free(device_props2);
    rox_options_free(options);
}

END_TEST

ROX_TEST_SUITE(
// DynamicApiTests
        ROX_TEST_CASE(test_is_enabled),
        ROX_TEST_CASE(test_is_enabled_after_setup),
        ROX_TEST_CASE(test_is_enabled_different_type_call),
        ROX_TEST_CASE(test_is_enabled_wrong_experiment_type),
        ROX_TEST_CASE(test_get_string),
        ROX_TEST_CASE(test_get_string_ignore_variation_null_when_variant_exists),
        ROX_TEST_CASE(test_get_string_different_type_call),
        ROX_TEST_CASE(test_get_int),
        ROX_TEST_CASE(test_get_int_different_type_call),
        ROX_TEST_CASE(test_get_int_wrong_experiment_type),
        ROX_TEST_CASE(test_get_double),
        ROX_TEST_CASE(test_get_double_different_type_call),
        ROX_TEST_CASE(test_get_double_wrong_experiment_type),
// InternalFlagsTests
        ROX_TEST_CASE(test_will_return_false_when_no_experiment),
        ROX_TEST_CASE(test_will_return_false_when_expression_is_null),
        ROX_TEST_CASE(test_will_return_false_when_expression_is_false),
        ROX_TEST_CASE(test_will_return_true_when_expression_is_true),
// MD5GeneratorTests
        ROX_TEST_CASE(test_will_check_md5_uses_right_props),
        ROX_TEST_CASE(test_will_check_md5_not_using_all_props),
        ROX_TEST_CASE(test_will_check_md5_with_objects),
        ROX_TEST_CASE(test_will_check_md5_with_json_object),
// BUIDTests
        ROX_TEST_CASE(test_will_generate_correct_md5_value)
)
