#include <assert.h>
#include <stdlib.h>
#include "roxtests.h"
#include "fixtures.h"
#include "core/impression/models.h"
#include "util.h"

typedef struct ServerTextContext {

    LoggingTestFixture *logging;

    //
    // Container
    //

    RoxVariant *simple_flag;
    RoxVariant *simple_flag_overwritten;
    RoxVariant *flag_for_impression;
    RoxVariant *flag_for_impression_with_experiment_and_context;
    RoxVariant *flag_custom_properties;
    RoxVariant *flag_target_groups_all;
    RoxVariant *flag_target_groups_any;
    RoxVariant *flag_target_groups_none;
    RoxVariant *variant_with_context;
    RoxVariant *variant;
    RoxVariant *variant_overwritten;
    RoxVariant *flag_for_dependency;
    RoxVariant *flag_colors_for_dependency;
    RoxVariant *flag_dependent;
    RoxVariant *flag_color_dependent_with_context;

    //
    // TestVars
    //

    bool is_impression_raised;
    bool is_computed_boolean_prop_called;
    bool is_computed_string_prop_called;
    bool is_computed_int_prop_called;
    bool is_computed_double_prop_called;
    bool is_computed_semver_prop_called;
    bool target_group1;
    bool target_group2;
    bool is_prop_for_target_group_for_dependency;

    int configuration_fetched_count;

    char *last_impression_value_name;
    char *last_impression_value;
    RoxExperiment *last_impression_experiment;
    RoxDynamicValue *last_impression_context_value;

} ServerTextContext;

static void _test_configuration_fetched_handler(void *target, RoxConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    ServerTextContext *ctx = (ServerTextContext *) target;
    if (args->fetcher_status == AppliedFromNetwork) {
        ++ctx->configuration_fetched_count;
    }
}

static void _test_rox_impression_handler(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context) {

    assert(target);

    if (!value) {
        return;
    }

    ServerTextContext *ctx = (ServerTextContext *) target;
    if (str_equals(value->name, "flagForImpression")) {
        ctx->is_impression_raised = true;
    }

    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
    }

    if (ctx->last_impression_value_name) {
        free(ctx->last_impression_value_name);
    }

    if (ctx->last_impression_experiment) {
        experiment_free(ctx->last_impression_experiment);
    }

    if (ctx->last_impression_context_value) {
        rox_dynamic_value_free(ctx->last_impression_context_value);
    }

    ctx->last_impression_value_name = mem_copy_str(value->name);
    ctx->last_impression_value = mem_copy_str(value->value);
    ctx->last_impression_experiment = experiment ? experiment_copy(experiment) : NULL;
    if (context) {
        RoxDynamicValue *val = rox_context_get(context, "var");
        ctx->last_impression_context_value = val ? rox_dynamic_value_create_copy(val) : NULL;
    } else {
        ctx->last_impression_context_value = NULL;
    }

}

static RoxDynamicValue *_test_computed_string_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->is_computed_string_prop_called = true;
    return rox_dynamic_value_create_string_copy("World");
}

static RoxDynamicValue *_test_computed_boolean_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->is_computed_boolean_prop_called = true;
    return rox_dynamic_value_create_boolean(false);
}

static RoxDynamicValue *_test_computed_property_using_context(void *target, RoxContext *context) {
    assert(target);
    assert(context);
    const char *key = (const char *) target;
    return rox_context_get(context, key);
}

static RoxDynamicValue *_test_computed_boolean_property_using_value(void *target, RoxContext *context) {
    assert(target);
    bool *value = (bool *) target;
    return rox_dynamic_value_create_boolean(*value);
}

static RoxDynamicValue *_test_computed_double_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->is_computed_double_prop_called = true;
    return rox_dynamic_value_create_double(1.618);
}

static RoxDynamicValue *_test_computed_semver_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->is_computed_semver_prop_called = true;
    return rox_dynamic_value_create_string_copy("20.7.1969");
}

static RoxDynamicValue *_test_computed_int_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->is_computed_int_prop_called = true;
    return rox_dynamic_value_create_int(28);
}

static ServerTextContext *_server_text_context_create() {
    ServerTextContext *ctx = calloc(1, sizeof(ServerTextContext));
    ctx->logging = logging_test_fixture_create(RoxLogLevelDebug);

    RoxOptions *options = rox_options_create();
    rox_options_set_configuration_fetched_handler(options, ctx, &_test_configuration_fetched_handler);
    rox_options_set_impression_handler(options, ctx, &_test_rox_impression_handler);
    rox_options_set_dev_mode_key(options, "37d6265f591155bb00ffb4e2");

    ctx->simple_flag = rox_add_flag("simpleFlag", true);
    ctx->simple_flag_overwritten = rox_add_flag("simpleFlagOverwritten", true);
    ctx->flag_for_impression = rox_add_flag("flagForImpression", false);
    ctx->flag_for_impression_with_experiment_and_context = rox_add_flag("flagForImpressionWithExperimentAndContext",
                                                                        false);
    ctx->flag_custom_properties = rox_add_flag("flagCustomProperties", false);
    ctx->flag_target_groups_all = rox_add_flag("flagTargetGroupsAll", false);
    ctx->flag_target_groups_any = rox_add_flag("flagTargetGroupsAny", false);
    ctx->flag_target_groups_none = rox_add_flag("flagTargetGroupsNone", false);
    ctx->variant_with_context = rox_add_variant("variantWithContext", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variant = rox_add_variant("variant", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variant_overwritten = rox_add_variant("variantOverwritten", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->flag_for_dependency = rox_add_flag("flagForDependency", false);
    ctx->flag_colors_for_dependency = rox_add_variant("flagColorsForDependency", "White",
                                                      ROX_LIST_COPY_STR("White", "Blue", "Green", "Yellow"));
    ctx->flag_dependent = rox_add_flag("flagDependent", false);
    ctx->flag_color_dependent_with_context = rox_add_variant("flagColorDependentWithContext", "White",
                                                             ROX_LIST_COPY_STR("White", "Blue", "Green", "Yellow"));

    rox_set_custom_string_property("stringProp1", "Hello");
    rox_set_custom_computed_string_property("stringProp2", ctx, &_test_computed_string_property);
    rox_set_custom_boolean_property("boolProp1", true);
    rox_set_custom_computed_boolean_property("boolProp2", ctx, &_test_computed_boolean_property);
    rox_set_custom_integer_property("intProp1", 6);
    rox_set_custom_computed_integer_property("intProp2", ctx, &_test_computed_int_property);
    rox_set_custom_double_property("doubleProp1", 3.14);
    rox_set_custom_computed_double_property("doubleProp2", ctx, &_test_computed_double_property);
    rox_set_custom_semver_property("smvrProp1", "9.11.2001");
    rox_set_custom_computed_semver_property("smvrProp2", ctx, &_test_computed_semver_property);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForVariant",
            "isDuckAndCover",
            &_test_computed_property_using_context);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForVariantDependency",
            "isDuckAndCover",
            &_test_computed_property_using_context);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupOperand1",
            &ctx->target_group1,
            &_test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupOperand2",
            &ctx->target_group2,
            &_test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForDependency",
            &ctx->is_prop_for_target_group_for_dependency,
            &_test_computed_boolean_property_using_value);

    rox_setup("5e579ecfc45c395c43b42893", options);

    return ctx;
}

static void _server_text_context_free(ServerTextContext *ctx) {
    assert(ctx);
    rox_shutdown();
    logging_test_fixture_free(ctx->logging);
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
    }
    if (ctx->last_impression_value_name) {
        free(ctx->last_impression_value_name);
    }
    if (ctx->last_impression_experiment) {
        experiment_free(ctx->last_impression_experiment);
    }
    if (ctx->last_impression_context_value) {
        rox_dynamic_value_free(ctx->last_impression_context_value);
    }
    free(ctx);
}

START_TEST (test_simple_flag) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(rox_flag_is_enabled(ctx->simple_flag));
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (test_simple_flag_overwritten) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(!rox_flag_is_enabled(ctx->simple_flag_overwritten));
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (test_variant) {
    ServerTextContext *ctx = _server_text_context_create();
    rox_check_and_free(rox_variant_get_value_or_default(ctx->variant), "red");
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (test_variant_overwritten) {
    ServerTextContext *ctx = _server_text_context_create();
    rox_check_and_free(rox_variant_get_value_or_default(ctx->variant_overwritten), "green");
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_all_custom_properties) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(rox_flag_is_enabled(ctx->flag_custom_properties));
    ck_assert(ctx->is_computed_boolean_prop_called);
    ck_assert(ctx->is_computed_double_prop_called);
    ck_assert(ctx->is_computed_int_prop_called);
    ck_assert(ctx->is_computed_semver_prop_called);
    ck_assert(ctx->is_computed_string_prop_called);
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_fetch_within_timeout) {
    ServerTextContext *ctx = _server_text_context_create();
    int number_of_config_fetches = ctx->configuration_fetched_count;
    rox_fetch(); // TODO: do a timeout 5000
    ck_assert_int_gt(ctx->configuration_fetched_count, number_of_config_fetches);
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_with_context) {
    ServerTextContext *ctx = _server_text_context_create();
    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    RoxContext *some_negative_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(false)));
    rox_check_and_free(rox_variant_get_value_or_default(ctx->variant_with_context), "red");
    rox_check_and_free(rox_variant_get_value_or_default_ctx(ctx->variant_with_context, some_positive_context), "blue");
    rox_check_and_free(rox_variant_get_value_or_default_ctx(ctx->variant_with_context, some_negative_context), "red");
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_target_groups_all_any_none) {
    ServerTextContext *ctx = _server_text_context_create();

    ctx->target_group1 = ctx->target_group2 = true;
    ck_assert(rox_flag_is_enabled(ctx->flag_target_groups_all));
    ck_assert(rox_flag_is_enabled(ctx->flag_target_groups_any));
    ck_assert(!rox_flag_is_enabled(ctx->flag_target_groups_none));

    ctx->target_group1 = false;
    ck_assert(!rox_flag_is_enabled(ctx->flag_target_groups_all));
    ck_assert(rox_flag_is_enabled(ctx->flag_target_groups_any));
    ck_assert(!rox_flag_is_enabled(ctx->flag_target_groups_none));

    ctx->target_group2 = false;
    ck_assert(!rox_flag_is_enabled(ctx->flag_target_groups_all));
    ck_assert(!rox_flag_is_enabled(ctx->flag_target_groups_any));
    ck_assert(rox_flag_is_enabled(ctx->flag_target_groups_none));

    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_impression_handler) {
    ServerTextContext *ctx = _server_text_context_create();

    rox_flag_is_enabled(ctx->flag_for_impression);
    ck_assert(ctx->is_impression_raised);
    ctx->is_impression_raised = false;

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("var"), rox_dynamic_value_create_string_copy("val")));
    bool flag_impression_value = rox_flag_is_enabled_ctx(ctx->flag_for_impression_with_experiment_and_context, context);
    ck_assert_ptr_nonnull(ctx->last_impression_value);
    ck_assert_str_eq("true", ctx->last_impression_value);
    ck_assert(flag_impression_value);
    ck_assert_str_eq("flagForImpressionWithExperimentAndContext", ctx->last_impression_value_name);

    ck_assert_ptr_nonnull(ctx->last_impression_experiment);
    ck_assert_str_eq("5e57a3d31d6e807bf3c307ee", ctx->last_impression_experiment->identifier);
    ck_assert_str_eq("flag for impression with experiment and context", ctx->last_impression_experiment->name);

    ck_assert_ptr_nonnull(ctx->last_impression_context_value);
    ck_assert(rox_dynamic_value_is_string(ctx->last_impression_context_value));
    ck_assert_str_eq(rox_dynamic_value_get_string(ctx->last_impression_context_value), "val");

    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_flag_dependency) {
    ServerTextContext *ctx = _server_text_context_create();

    ctx->is_prop_for_target_group_for_dependency = true;
    ck_assert(rox_flag_is_enabled(ctx->flag_for_dependency));
    ck_assert(!rox_flag_is_enabled(ctx->flag_dependent));

    ctx->is_prop_for_target_group_for_dependency = false;
    ck_assert(rox_flag_is_enabled(ctx->flag_dependent));
    ck_assert(!rox_flag_is_enabled(ctx->flag_for_dependency));

    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_dependency_with_context) {
    ServerTextContext *ctx = _server_text_context_create();

    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    RoxContext *some_negative_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(false)));

    rox_check_and_free(
            rox_variant_get_value_or_default(ctx->flag_color_dependent_with_context),
            "White");

    rox_check_and_free(
            rox_variant_get_value_or_default_ctx(ctx->flag_color_dependent_with_context, some_negative_context),
            "White");

    rox_check_and_free(
            rox_variant_get_value_or_default_ctx(ctx->flag_color_dependent_with_context, some_positive_context),
            "Yellow");

    _server_text_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_simple_flag),
        ROX_TEST_CASE(test_simple_flag_overwritten),
        ROX_TEST_CASE(test_variant),
        ROX_TEST_CASE(test_variant_overwritten),
        ROX_TEST_CASE(testing_all_custom_properties),
        ROX_TEST_CASE(testing_fetch_within_timeout),
        ROX_TEST_CASE(testing_variant_with_context),
        ROX_TEST_CASE(testing_target_groups_all_any_none),
        ROX_TEST_CASE(testing_impression_handler),
        ROX_TEST_CASE(testing_flag_dependency),
        ROX_TEST_CASE(testing_variant_dependency_with_context)
)
