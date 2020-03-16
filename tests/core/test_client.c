#include <stdbool.h>
#include <core/consts.h>
#include <assert.h>
#include "roxtests.h"
#include "roxx/parser.h"
#include "core/repositories.h"
#include "core/client.h"
#include "core/entities.h"
#include "collections.h"

//
// DynamicApiTests
//

typedef struct TestImpressionHandler {
    char *last_impression_value;
    int impressions;
} TestImpressionHandler;

static void _test_impression_handler_func(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context) {

    TestImpressionHandler *handler = (TestImpressionHandler *) target;
    if (handler->last_impression_value) {
        free(handler->last_impression_value);
    }
    handler->last_impression_value = value->value ? mem_copy_str(value->value) : NULL;
    ++handler->impressions;
}

static TestImpressionHandler *_test_impression_handler_create() {
    return calloc(1, sizeof(TestImpressionHandler));
}

static void _test_impression_handler_free(TestImpressionHandler *handler) {
    assert(handler);
    if (handler->last_impression_value) {
        free(handler->last_impression_value);
    }
    free(handler);
}

START_TEST (test_is_enabled) {

    TestImpressionHandler *impression_handler = _test_impression_handler_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();
    impression_invoker_register(impression_invoker, impression_handler, &_test_impression_handler_func);

    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    EntitiesProvider *entities_provider = entities_provider_create();
    RoxDynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", true, NULL));
    ck_assert_str_eq(impression_handler->last_impression_value, "true");
    ck_assert_int_eq(impression_handler->impressions, 1);

    ck_assert(flag_is_enabled(flag_repository_get_flag(flag_repo, "default.newFlag"), NULL));
    ck_assert_str_eq(impression_handler->last_impression_value, "true");
    ck_assert_int_eq(impression_handler->impressions, 2);

    ck_assert(!dynamic_api_is_enabled(api, "default.newFlag", false, NULL));
    ck_assert_str_eq(impression_handler->last_impression_value, "false");
    ck_assert_int_eq(impression_handler->impressions, 3);

    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(flag_repo)));

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newFlag", "and(true, true)", false,
                                    ROX_LIST_COPY_STR("default.newFlag"), ROX_EMPTY_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", false, NULL));
    ck_assert_str_eq(impression_handler->last_impression_value, "true");
    ck_assert_int_eq(impression_handler->impressions, 4);

    rox_dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);

    impression_invoker_free(impression_invoker);
    _test_impression_handler_free(impression_handler);
}

END_TEST

START_TEST (test_is_enabled_after_setup) {
    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, NULL);
    EntitiesProvider *entities_provider = entities_provider_create();
    RoxDynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newFlag", "and(true, true)", false,
                                    ROX_LIST_COPY_STR("default.newFlag"), ROX_EMPTY_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", false, NULL));

    rox_dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
}

END_TEST

START_TEST (test_get_value) {

    TestImpressionHandler *impression_handler = _test_impression_handler_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();
    impression_invoker_register(impression_invoker, impression_handler, &_test_impression_handler_func);

    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    EntitiesProvider *entities_provider = entities_provider_create();
    RoxDynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    RoxList *options = ROX_LIST_COPY_STR("A", "B", "C");
    rox_check_and_free(rox_dynamic_api_get_value(api, "default.newVariant", "A", options, NULL), "A");
    ck_assert_str_eq(impression_handler->last_impression_value, "A");
    ck_assert_int_eq(impression_handler->impressions, 1);

    RoxVariant *flag = flag_repository_get_flag(flag_repo, "default.newVariant");
    rox_check_and_free(variant_get_value_or_default(flag, NULL), "A");
    ck_assert_str_eq(impression_handler->last_impression_value, "A");
    ck_assert_int_eq(impression_handler->impressions, 2);

    rox_check_and_free(rox_dynamic_api_get_value(api, "default.newVariant", "B", options, NULL), "B");
    ck_assert_int_eq(1, rox_map_size(flag_repository_get_all_flags(flag_repo)));
    ck_assert_str_eq(impression_handler->last_impression_value, "B");
    ck_assert_int_eq(impression_handler->impressions, 3);

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newVariant", "ifThen(true, \"B\", \"A\")", false,
                                    ROX_LIST_COPY_STR("default.newVariant"), ROX_EMPTY_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    rox_check_and_free(rox_dynamic_api_get_value(api, "default.newVariant", "A", options, NULL), "B");
    ck_assert_str_eq(impression_handler->last_impression_value, "B");
    ck_assert_int_eq(impression_handler->impressions, 4);

    rox_dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
    _test_impression_handler_free(impression_handler);
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
        ROX_TEST_CASE(test_get_value),
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
