#include <stdbool.h>
#include <core/consts.h>
#include "roxtests.h"
#include "roxx/parser.h"
#include "core/repositories.h"
#include "core/client.h"
#include "core/entities.h"
#include "util.h"

//
// DynamicApiTests
//

START_TEST (test_is_enabled) {
    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, NULL);
    EntitiesProvider *entities_provider = entities_provider_create();
    DynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", true, NULL));
    ck_assert(flag_is_enabled(flag_repository_get_flag(flag_repo, "default.newFlag"), NULL));
    ck_assert(!dynamic_api_is_enabled(api, "default.newFlag", false, NULL));
    ck_assert_int_eq(1, hashtable_size(flag_repository_get_all_flags(flag_repo)));

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newFlag", "and(true, true)", false,
                                    ROX_LIST_COPY_STR("default.newFlag"), ROX_EMPTY_HASH_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", false, NULL));

    dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
}

END_TEST

START_TEST (test_is_enabled_after_setup) {
    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, NULL);
    EntitiesProvider *entities_provider = entities_provider_create();
    DynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newFlag", "and(true, true)", false,
                                    ROX_LIST_COPY_STR("default.newFlag"), ROX_EMPTY_HASH_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    ck_assert(dynamic_api_is_enabled(api, "default.newFlag", false, NULL));

    dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
}

END_TEST

START_TEST (test_get_value) {
    Parser *parser = parser_create();
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, NULL);
    EntitiesProvider *entities_provider = entities_provider_create();
    DynamicApi *api = dynamic_api_create(flag_repo, entities_provider);

    List *options = ROX_LIST_COPY_STR("A", "B", "C");
    check_and_free(dynamic_api_get_value(api, "default.newVariant", "A", options, NULL), "A");

    Variant *flag = flag_repository_get_flag(flag_repo, "default.newVariant");
    check_and_free(variant_get_value_or_default(flag, NULL), "A");

    check_and_free(dynamic_api_get_value(api, "default.newVariant", "B", options, NULL), "B");
    ck_assert_int_eq(1, hashtable_size(flag_repository_get_all_flags(flag_repo)));

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "default.newVariant", "ifThen(true, \"B\", \"A\")", false,
                                    ROX_LIST_COPY_STR("default.newVariant"), ROX_EMPTY_HASH_SET, "stam")
    ));
    flag_setter_set_experiments(flag_setter);

    check_and_free(dynamic_api_get_value(api, "default.newVariant", "A", options, NULL), "B");

    dynamic_api_free(api);
    entities_provider_free(entities_provider);
    flag_setter_free(flag_setter);
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

START_TEST (test_will_check_m_d5_uses_right_props) {
    HashTable *props;
    hashtable_new(&props);
    hashtable_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "plat");
    char *md5 = md5_generator_generate(props, ROX_LIST(&ROX_PROPERTY_TYPE_PLATFORM), NULL);
    check_and_free(md5, "1380AFEBC7CE22DE7B3450F8CAB86D2C");
    hashtable_destroy(props);
}

END_TEST

START_TEST (test_will_check_m_d5_not_using_all_props) {
    HashTable *props;
    hashtable_new(&props);
    hashtable_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "dev");
    hashtable_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "plat");
    char *md5 = md5_generator_generate(props, ROX_LIST(&ROX_PROPERTY_TYPE_PLATFORM), NULL);
    check_and_free(md5, "1380AFEBC7CE22DE7B3450F8CAB86D2C");
    hashtable_destroy(props);
}

END_TEST

START_TEST (test_will_check_m_d5_with_objects) {
    HashTable *props;
    hashtable_new(&props);
    hashtable_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "22");
    hashtable_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "True"); // Dotnet legacy, true becomes True
    char *md5 = md5_generator_generate(props, ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET), NULL);
    check_and_free(md5, "D3816631EDE04D536EAEB479FE5829FD");
    hashtable_destroy(props);
}

END_TEST

START_TEST (test_will_check_m_d5_with_j_s_o_n_object) {
    HashTable *props;
    hashtable_new(&props);
    hashtable_add(props, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name, "[{\"key\":\"value\"}]");
    hashtable_add(props, ROX_PROPERTY_TYPE_PLATFORM.name, "value"); // Dotnet legacy, true becomes True
    char *md5 = md5_generator_generate(props, ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET), NULL);
    check_and_free(md5, "AA16F2AA33D095940A93C991B00D55C7");
    hashtable_destroy(props);
}

END_TEST

//
// BUIDTests
//

START_TEST (test_will_generate_correct_m_d5_value) {
    SdkSettings sdk_settings = {"test", "test"};
    RoxOptions *options = rox_options_create();
    DeviceProperties *device_props = device_properties_create_from_map(&sdk_settings, options, ROX_HASH_TABLE(
            "app_key", ROX_COPY("123"),
            "api_version", ROX_COPY("4.0.0"),
            "platform", ROX_COPY("plat"),
            "lib_version", ROX_COPY("1.5.0")
    ));

    BUID *buid = buid_create(device_props);
    check_and_free(buid_get_value(buid), "234A32BB4341EAFD91FC8D0395F4E66F");
    device_properties_free(device_props);
    buid_free(buid);

    DeviceProperties *device_props2 = device_properties_create_from_map(&sdk_settings, options, ROX_HASH_TABLE(
            "app_key", ROX_COPY("122"),
            "api_version", ROX_COPY("4.0.0"),
            "platform", ROX_COPY("plat"),
            "lib_version", ROX_COPY("1.5.0")
    ));

    BUID *buid2 = buid_create(device_props2);
    check_and_free(buid_get_value(buid2), "F5F30C84B8A806E0004043864724A56E");
    buid_free(buid2);
    device_properties_free(device_props2);
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
        ROX_TEST_CASE(test_will_check_m_d5_uses_right_props),
        ROX_TEST_CASE(test_will_check_m_d5_not_using_all_props),
        ROX_TEST_CASE(test_will_check_m_d5_with_objects),
        ROX_TEST_CASE(test_will_check_m_d5_with_j_s_o_n_object),
// BUIDTests
        ROX_TEST_CASE(test_will_generate_correct_m_d5_value)
)
