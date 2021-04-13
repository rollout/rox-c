#include <assert.h>
#include <stdlib.h>
#include "roxtests.h"
#include "fixtures.h"
#include "util.h"

// FlagTests

START_TEST (test_flag_with_default_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", true);
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_with_default_value_after_setup) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", false);
    ck_assert(!rox_is_enabled(flag));
    flag_test_fixture_check_impression(ctx, "false");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_with_experiment_expression_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", false);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "and(true, true)"));
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(ctx, "true");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_with_experiment_returns_undefined) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", true);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "undefined"));
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(ctx, "true");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_with_experiment_wrong_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", true);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "0"));
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(ctx, "true");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_will_use_context) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    ctx->imp_context_key = "key";
    RoxStringBase *flag = rox_add_flag("name", false);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "true"));
    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("key"), rox_dynamic_value_create_int(55)));
    ck_assert(rox_is_enabled_ctx(flag, context));
    ck_assert_int_eq(55, rox_dynamic_value_get_int(ctx->imp_context_value));
    flag_test_fixture_check_impression(ctx, "true");
    flag_test_fixture_free(ctx);
    rox_context_free(context);
}

END_TEST

START_TEST (test_flag_will_invoke_enabled_action) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", true);
    flag_test_fixture_handle_enabled_callback(ctx, flag);
    ck_assert(ctx->test_flag_action_called);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_flag_will_invoke_disabled_action) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("name", false);
    flag_test_fixture_handle_disable_callback(ctx, flag);
    ck_assert(ctx->test_flag_action_called);
    flag_test_fixture_free(ctx);
}

END_TEST

// RoxStringTests

START_TEST (test_string_will_add_default_to_options_when_no_options) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "1");
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 1);
    ck_assert(str_in_list("1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_not_add_default_to_options_if_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string_with_options("name", "1", ROX_LIST_COPY_STR("1", "2", "3"));
    ck_assert_int_eq(rox_list_size(variant_get_options(variant)), 3);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_add_default_to_options_if_not_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string_with_options("name", "1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_int_eq(rox_list_size(variant_get_options(variant)), 3);
    ck_assert(str_in_list("1", variant_get_options(variant)));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_set_name) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "1");
    ck_assert_str_eq(variant_get_name(variant), "name");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_default_no_experiment) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "val");
    rox_check_and_free(rox_get_string(variant), "val");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_default_after_setup) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "val");
    rox_check_and_free(rox_get_string(variant), "val");
    flag_test_fixture_check_impression(ctx, "val");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_with_experiment) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "val");
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "\"dif\""));
    rox_check_and_free(rox_get_string(variant), "dif");
    flag_test_fixture_check_impression(ctx, "dif");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_with_experiment_returns_undefined) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_string("name", "val");
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "undefined"));
    rox_check_and_free(rox_get_string(variant), "val");
    flag_test_fixture_check_impression(ctx, "val");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_string_will_use_context) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    ctx->imp_context_key = "key";

    RoxStringBase *variant = rox_add_string("name", "val");
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "\"dif\""));

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("key"), rox_dynamic_value_create_int(55)));

    rox_check_and_free(rox_get_string_ctx(variant, context), "dif");
    flag_test_fixture_check_impression(ctx, "dif");
    ck_assert(ctx->imp_context_value);
    ck_assert_int_eq(55, rox_dynamic_value_get_int(ctx->imp_context_value));

    rox_context_free(context);
    flag_test_fixture_free(ctx);
}

END_TEST

// RoxIntTests

START_TEST (test_int_will_add_default_to_options_when_no_options) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 1);
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 1);
    ck_assert(str_in_list("1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_not_add_default_to_options_if_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int_with_options("name", 1, ROX_INT_LIST(1, 2, 3));
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 3);
    ck_assert(str_in_list("1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_add_default_to_options_if_not_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int_with_options("name", 1, ROX_INT_LIST(2, 3));
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 3);
    ck_assert(str_in_list("1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_return_default_when_no_experiment) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 3);
    ck_assert_int_eq(3, rox_get_int(variant));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_return_default_when_no_experiment_after_setup) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 3);
    ck_assert_int_eq(3, rox_get_int(variant));
    flag_test_fixture_check_impression(ctx, "3");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_return_default_when_experiment_returns_undefined) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 3);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "undefined"));
    ck_assert_int_eq(3, rox_get_int(variant));
    flag_test_fixture_check_impression(ctx, "3");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_return_experiment_expression_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 1);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "2"));
    ck_assert_int_eq(2, rox_get_int(variant));
    flag_test_fixture_check_impression(ctx, "2");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_return_default_when_wrong_experiment_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_int("name", 4);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "1.44"));
    ck_assert_int_eq(4, rox_get_int(variant));
    flag_test_fixture_check_impression(ctx, "4");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_int_will_use_context) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    ctx->imp_context_key = "key";

    RoxStringBase *variant = rox_add_int_with_options("name", 1, ROX_INT_LIST(2, 3));
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "2"));

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("key"), rox_dynamic_value_create_int(55)));

    ck_assert_int_eq(2, rox_get_int_ctx(variant, context));
    ck_assert(ctx->imp_context_value);
    ck_assert_int_eq(55, rox_dynamic_value_get_int(ctx->imp_context_value));
    flag_test_fixture_check_impression(ctx, "2");

    rox_context_free(context);
    flag_test_fixture_free(ctx);
}

END_TEST

// RoxDoubleTests

START_TEST (test_double_will_add_default_to_options_when_no_options) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 1);
    ck_assert(str_in_list("1.1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_not_add_default_to_options_if_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double_with_options("name", 1.1, ROX_DBL_LIST(1.1, 2., 3.));
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 3);
    ck_assert(str_in_list("1.1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_add_default_to_options_if_not_exists) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double_with_options("name", 1.1, ROX_DBL_LIST(2., 3.));
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 3);
    ck_assert(str_in_list("1.1", list));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_return_default) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    ck_assert_double_eq(1.1, rox_get_double(variant));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_return_default_when_no_experiment_after_setup) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    ck_assert_double_eq(1.1, rox_get_double(variant));
    check_impression_ex(ctx, "1.1", false);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_return_default_when_experiment_returns_undefined) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "undefined"));
    ck_assert_double_eq(1.1, rox_get_double(variant));
    check_impression_ex(ctx, "1.1", true);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_return_experiment_expression_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "2.2"));
    ck_assert_double_eq(2.2, rox_get_double(variant));
    flag_test_fixture_check_impression(ctx, "2.2");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_return_default_when_experiment_wrong_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *variant = rox_add_double("name", 1.1);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "2ss"));
    ck_assert_double_eq(1.1, rox_get_double(variant));
    check_impression_ex(ctx, "1.1", true);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_double_will_use_context) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    ctx->imp_context_key = "key";

    RoxStringBase *variant = rox_add_double_with_options("name", 1.1, ROX_DBL_LIST(2., 3.));
    flag_test_fixture_set_experiments(ctx, ROX_MAP("name", "2.2"));

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("key"), rox_dynamic_value_create_int(55)));

    ck_assert_double_eq(2.2, rox_get_double_ctx(variant, context));
    ck_assert(ctx->imp_context_value);
    ck_assert_int_eq(55, rox_dynamic_value_get_int(ctx->imp_context_value));
    check_impression_ex(ctx, "2.2", true);

    rox_context_free(context);
    flag_test_fixture_free(ctx);
}

END_TEST

// RoxE2ETests

typedef struct ServerTestContext {

    LoggingTestFixture *logging;

    //
    // Container
    //

    RoxStringBase *simpleFlag;
    RoxStringBase *simpleFlagOverwritten;
    RoxStringBase *flagForImpression;
    RoxStringBase *flagForImpressionWithExperimentAndContext;
    RoxStringBase *flagCustomProperties;
    RoxStringBase *flagTargetGroupsAll;
    RoxStringBase *flagTargetGroupsAny;
    RoxStringBase *flagTargetGroupsNone;
    RoxStringBase *variantWithContext;
    RoxStringBase *variant;
    RoxStringBase *variantOverwritten;
    RoxStringBase *flagForDependency;
    RoxStringBase *flag_colors_for_dependency;
    RoxStringBase *flagDependent;
    RoxStringBase *flagColorDependentWithContext;

    //
    // TestVars
    //

    bool isImpressionRaised;
    bool isComputedBooleanPropCalled;
    bool isComputedStringPropCalled;
    bool isComputedIntPropCalled;
    bool isComputedDoublePropCalled;
    bool isComputedSemverPropCalled;
    bool targetGroup1;
    bool targetGroup2;
    bool isPropForTargetGroupForDependency;

    int configurationFetchedCount;
    RoxFetcherError last_error;
    RoxFetchStatus last_status;

    char *lastImpressionValueName;
    char *lastImpressionValue;
    RoxDynamicValue *lastImpressionContextValue;

} ServerTestContext;

static void test_configuration_fetched_handler(void *target, RoxConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    ServerTestContext *ctx = (ServerTestContext *) target;
    if (args) {
        ctx->last_status = args->fetcher_status;
        ctx->last_error = args->error_details;
        if (args->error_details == NoError) {
            ++ctx->configurationFetchedCount;
        }
    }
}

static void test_rox_impression_handler(
        void *target,
        RoxReportingValue *value,
        RoxContext *context) {

    assert(target);

    if (!value) {
        return;
    }

    ServerTestContext *ctx = (ServerTestContext *) target;
    if (str_equals(value->name, "flagForImpression")) {
        ctx->isImpressionRaised = true;
    }

    if (ctx->lastImpressionValue) {
        free(ctx->lastImpressionValue);
    }

    if (ctx->lastImpressionValueName) {
        free(ctx->lastImpressionValueName);
    }

    if (ctx->lastImpressionContextValue) {
        rox_dynamic_value_free(ctx->lastImpressionContextValue);
    }

    ctx->lastImpressionValueName = mem_copy_str(value->name);
    ctx->lastImpressionValue = mem_copy_str(value->value);
    if (context) {
        ctx->lastImpressionContextValue = rox_context_get(context, "var");
    } else {
        ctx->lastImpressionContextValue = NULL;
    }

}

static RoxDynamicValue *test_computed_string_property(void *target, RoxContext *context) {
    ServerTestContext *ctx = (ServerTestContext *) target;
    ctx->isComputedStringPropCalled = true;
    return rox_dynamic_value_create_string_copy("World");
}

static RoxDynamicValue *test_computed_boolean_property(void *target, RoxContext *context) {
    ServerTestContext *ctx = (ServerTestContext *) target;
    ctx->isComputedBooleanPropCalled = true;
    return rox_dynamic_value_create_boolean(false);
}

static RoxDynamicValue *test_computed_property_using_context(void *target, RoxContext *context) {
    assert(target);
    assert(context);
    const char *key = (const char *) target;
    return rox_context_get(context, key);
}

static RoxDynamicValue *test_computed_boolean_property_using_value(void *target, RoxContext *context) {
    assert(target);
    bool *value = (bool *) target;
    return rox_dynamic_value_create_boolean(*value);
}

static RoxDynamicValue *test_computed_double_property(void *target, RoxContext *context) {
    ServerTestContext *ctx = (ServerTestContext *) target;
    ctx->isComputedDoublePropCalled = true;
    return rox_dynamic_value_create_double(1.618);
}

static RoxDynamicValue *test_computed_semver_property(void *target, RoxContext *context) {
    ServerTestContext *ctx = (ServerTestContext *) target;
    ctx->isComputedSemverPropCalled = true;
    return rox_dynamic_value_create_string_copy("20.7.1969");
}

static RoxDynamicValue *test_computed_int_property(void *target, RoxContext *context) {
    ServerTestContext *ctx = (ServerTestContext *) target;
    ctx->isComputedIntPropCalled = true;
    return rox_dynamic_value_create_int(28);
}

typedef struct ServerTestContextConfig {
    RoxContext *context;
#ifdef ROX_CLIENT
    RoxMap *storage_values;
#endif
} ServerTestContextConfig;

static ServerTestContext *server_test_context_create_cfg(ServerTestContextConfig *config) {
    ServerTestContext *ctx = calloc(1, sizeof(ServerTestContext));
    ctx->logging = logging_test_fixture_create(RoxLogLevelDebug);

    RoxOptions *options = rox_options_create();
    rox_options_set_configuration_fetched_handler(options, ctx, &test_configuration_fetched_handler);
    rox_options_set_impression_handler(options, ctx, &test_rox_impression_handler);
    rox_options_set_dev_mode_key(options, "37d6265f591155bb00ffb4e2");

#ifdef ROX_CLIENT

    if (config && config->storage_values) {
        set_in_memory_storage_config_with_values(options, config->storage_values);
    } else {
        set_dummy_storage_config(options);
    }

#endif

    bool setup_called = false;
    const char *app_key = "5e579ecfc45c395c43b42893";
    if (config && config->context) {
        rox_setup(app_key, options);
        rox_set_context(config->context);
        setup_called = true;
    }

    ctx->simpleFlag = rox_add_flag("simpleFlag", true);
    ctx->simpleFlagOverwritten = rox_add_flag("simpleFlagOverwritten", true);
    ctx->flagForImpression = rox_add_flag("flagForImpression", false);
    ctx->flagForImpressionWithExperimentAndContext = rox_add_flag("flagForImpressionWithExperimentAndContext",
                                                                  false);
    ctx->flagCustomProperties = rox_add_flag("flagCustomProperties", false);
    ctx->flagTargetGroupsAll = rox_add_flag("flagTargetGroupsAll", false);
    ctx->flagTargetGroupsAny = rox_add_flag("flagTargetGroupsAny", false);
    ctx->flagTargetGroupsNone = rox_add_flag("flagTargetGroupsNone", false);
    ctx->variantWithContext = rox_add_string_with_options("variantWithContext", "red",
                                                          ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variant = rox_add_string_with_options("variant", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variantOverwritten = rox_add_string_with_options("variantOverwritten", "red",
                                                          ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->flagForDependency = rox_add_flag("flagForDependency", false);
    ctx->flag_colors_for_dependency = rox_add_string_with_options("flagColorsForDependency", "White",
                                                                  ROX_LIST_COPY_STR("White", "Blue", "Green",
                                                                                    "Yellow"));
    ctx->flagDependent = rox_add_flag("flagDependent", false);
    ctx->flagColorDependentWithContext = rox_add_string_with_options("flagColorDependentWithContext", "White",
                                                                     ROX_LIST_COPY_STR("White", "Blue", "Green",
                                                                                       "Yellow"));

    rox_set_custom_string_property("stringProp1", "Hello");
    rox_set_custom_computed_string_property("stringProp2", ctx, &test_computed_string_property);
    rox_set_custom_boolean_property("boolProp1", true);
    rox_set_custom_computed_boolean_property("boolProp2", ctx, &test_computed_boolean_property);
    rox_set_custom_integer_property("intProp1", 6);
    rox_set_custom_computed_integer_property("intProp2", ctx, &test_computed_int_property);
    rox_set_custom_double_property("doubleProp1", 3.14);
    rox_set_custom_computed_double_property("doubleProp2", ctx, &test_computed_double_property);
    rox_set_custom_semver_property("smvrProp1", "9.11.2001");
    rox_set_custom_computed_semver_property("smvrProp2", ctx, &test_computed_semver_property);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForVariant",
            "isDuckAndCover",
            &test_computed_property_using_context);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForVariantDependency",
            "isDuckAndCover",
            &test_computed_property_using_context);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupOperand1",
            &ctx->targetGroup1,
            &test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupOperand2",
            &ctx->targetGroup2,
            &test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForDependency",
            &ctx->isPropForTargetGroupForDependency,
            &test_computed_boolean_property_using_value);

    if (!setup_called) {
        rox_setup(app_key, options);
    }

    return ctx;
}

static ServerTestContext *server_test_context_create() {
    return server_test_context_create_cfg(NULL);
}

static void server_test_context_free(ServerTestContext *ctx) {
    assert(ctx);
    rox_shutdown();
    logging_test_fixture_free(ctx->logging);
    if (ctx->lastImpressionValue) {
        free(ctx->lastImpressionValue);
    }
    if (ctx->lastImpressionValueName) {
        free(ctx->lastImpressionValueName);
    }
    if (ctx->lastImpressionContextValue) {
        rox_dynamic_value_free(ctx->lastImpressionContextValue);
    }
    free(ctx);
}

START_TEST (test_simple_flag) {
    ServerTestContext *ctx = server_test_context_create();
    ck_assert(rox_is_enabled(ctx->simpleFlag));
    server_test_context_free(ctx);
}

END_TEST

START_TEST (test_simple_flag_overwritten) {
    ServerTestContext *ctx = server_test_context_create();
    ck_assert(!rox_is_enabled(ctx->simpleFlagOverwritten));
    server_test_context_free(ctx);
}

END_TEST

START_TEST (test_variant) {
    ServerTestContext *ctx = server_test_context_create();
    rox_check_and_free(rox_get_string(ctx->variant), "red");
    server_test_context_free(ctx);
}

END_TEST

START_TEST (test_variant_overwritten) {
    ServerTestContext *ctx = server_test_context_create();
    rox_check_and_free(rox_get_string(ctx->variantOverwritten), "green");
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_all_custom_properties) {
    ServerTestContext *ctx = server_test_context_create();
    ck_assert(rox_is_enabled(ctx->flagCustomProperties));
    ck_assert(ctx->isComputedBooleanPropCalled);
    ck_assert(ctx->isComputedDoublePropCalled);
    ck_assert(ctx->isComputedIntPropCalled);
    ck_assert(ctx->isComputedSemverPropCalled);
    ck_assert(ctx->isComputedStringPropCalled);
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_fetch_within_timeout) {
    ServerTestContext *ctx = server_test_context_create();
    int number_of_config_fetches = ctx->configurationFetchedCount;
    rox_fetch(); // TODO: do a timeout 5000
    ck_assert_int_gt(ctx->configurationFetchedCount, number_of_config_fetches);
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_with_context) {
    ServerTestContext *ctx = server_test_context_create();
    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    RoxContext *some_negative_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(false)));
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "red");
    rox_check_and_free(rox_get_string_ctx(ctx->variantWithContext, some_positive_context), "blue");
    rox_check_and_free(rox_get_string_ctx(ctx->variantWithContext, some_negative_context), "red");
    rox_context_free(some_positive_context);
    rox_context_free(some_negative_context);
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_with_global_context) {
    ServerTestContext *ctx = server_test_context_create();
    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "red");
    rox_set_context(some_positive_context);
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "blue");
    rox_set_context(NULL);
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "red");
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_with_global_context_set_before) {
    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    ServerTestContextConfig config = {some_positive_context};
    ServerTestContext *ctx = server_test_context_create_cfg(&config);
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "blue");
    rox_set_context(NULL);
    rox_check_and_free(rox_get_string(ctx->variantWithContext), "red");
    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_target_groups_all_any_none) {
    ServerTestContext *ctx = server_test_context_create();

    ctx->targetGroup1 = ctx->targetGroup2 = true;
    ck_assert(rox_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(rox_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(!rox_is_enabled(ctx->flagTargetGroupsNone));

    ctx->targetGroup1 = false;
    ck_assert(!rox_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(rox_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(!rox_is_enabled(ctx->flagTargetGroupsNone));

    ctx->targetGroup2 = false;
    ck_assert(!rox_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(!rox_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(rox_is_enabled(ctx->flagTargetGroupsNone));

    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_impression_handler) {
    ServerTestContext *ctx = server_test_context_create();

    rox_is_enabled(ctx->flagForImpression);
    ck_assert(ctx->isImpressionRaised);
    ctx->isImpressionRaised = false;

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("var"), rox_dynamic_value_create_string_copy("val")));
    bool flag_impression_value = rox_is_enabled_ctx(ctx->flagForImpressionWithExperimentAndContext, context);
    ck_assert_ptr_nonnull(ctx->lastImpressionValue);
    ck_assert_str_eq("true", ctx->lastImpressionValue);
    ck_assert(flag_impression_value);
    ck_assert_str_eq("flagForImpressionWithExperimentAndContext", ctx->lastImpressionValueName);

    ck_assert_ptr_nonnull(ctx->lastImpressionContextValue);
    ck_assert(rox_dynamic_value_is_string(ctx->lastImpressionContextValue));
    ck_assert_str_eq(rox_dynamic_value_get_string(ctx->lastImpressionContextValue), "val");

    rox_context_free(context);

    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_flag_dependency) {
    ServerTestContext *ctx = server_test_context_create();

    ctx->isPropForTargetGroupForDependency = true;
    ck_assert(rox_is_enabled(ctx->flagForDependency));
    ck_assert(!rox_is_enabled(ctx->flagDependent));

    ctx->isPropForTargetGroupForDependency = false;
    ck_assert(rox_is_enabled(ctx->flagDependent));
    ck_assert(!rox_is_enabled(ctx->flagForDependency));

    server_test_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_dependency_with_context) {
    ServerTestContext *ctx = server_test_context_create();

    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    RoxContext *some_negative_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(false)));

    rox_check_and_free(
            rox_get_string(ctx->flagColorDependentWithContext),
            "White");

    rox_check_and_free(
            rox_get_string_ctx(ctx->flagColorDependentWithContext, some_negative_context),
            "White");

    rox_check_and_free(
            rox_get_string_ctx(ctx->flagColorDependentWithContext, some_positive_context),
            "Yellow");

    rox_context_free(some_positive_context);
    rox_context_free(some_negative_context);

    server_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_use_local_cache) {
#ifdef ROX_CLIENT
    const char *config_data = "{\"data\":\"{\\\"data\\\":\\\"{\\\\\\\"experiments\\\\\\\":[{\\\\\\\"_id\\\\\\\":\\\\\\\"5b322d790d81206da3055831\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"configuration fetched\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"rox.internal.configurationFetched\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\")\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5b45ce19bddf203f97fdb82a\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"analytics\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"rox.internal.analytics\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\")\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.internal.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5be03e1bfbb5900464c5feac\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"push updates\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"rox.internal.pushUpdates\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\")\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d01d6e80f485c307b8\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag color dependent with context\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagColorDependentWithContext\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, eq(flagValue(\\\\\\\\\\\\\\\"flagColorsForDependency\\\\\\\\\\\\\\\"), b64d(\\\\\\\\\\\\\\\"R3JlZW4=\\\\\\\\\\\\\\\"))), b64d(\\\\\\\\\\\\\\\"WWVsbG93\\\\\\\\\\\\\\\"), ifThen(and(true, true), undefined, undefined))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d11d6e8048e1c307c2\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag colors for dependency\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagColorsForDependency\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d11d6e8060c5c307bb\\\\\\\\\\\\\\\")), b64d(\\\\\\\\\\\\\\\"R3JlZW4=\\\\\\\\\\\\\\\"), ifThen(and(true, true), undefined, undefined))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d11d6e80d0cfc307ce\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag custom properties\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagCustomProperties\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d11d6e80596ac307c5\\\\\\\\\\\\\\\")), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d21d6e800355c307da\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag dependent\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagDependent\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, eq(flagValue(\\\\\\\\\\\\\\\"flagForDependency\\\\\\\\\\\\\\\"), b64d(\\\\\\\\\\\\\\\"dHJ1ZQ==\\\\\\\\\\\\\\\"))), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d21d6e80f381c307e6\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag for dependency\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagForDependency\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d21d6e80022bc307dd\\\\\\\\\\\\\\\")), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d31d6e807bf3c307ee\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag for impression with experiment and context\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagForImpressionWithExperimentAndContext\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\")\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d31d6e80af19c307fc\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag target groups all\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagTargetGroupsAll\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, and(isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e805829c307f1\\\\\\\\\\\\\\\"), isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e806ec6c307f3\\\\\\\\\\\\\\\"))), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d41d6e805f53c30808\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag target groups any\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagTargetGroupsAny\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, or(isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e805829c307f1\\\\\\\\\\\\\\\"), isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e806ec6c307f3\\\\\\\\\\\\\\\"))), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d41d6e80ae51c30812\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"flag target groups none\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"flagTargetGroupsNone\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, not(or(isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e805829c307f1\\\\\\\\\\\\\\\"), isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d31d6e806ec6c307f3\\\\\\\\\\\\\\\")))), \\\\\\\\\\\\\\\"true\\\\\\\\\\\\\\\", ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\"))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d51d6e804d52c30819\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"simple flag overwritten\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"simpleFlagOverwritten\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\", \\\\\\\\\\\\\\\"false\\\\\\\\\\\\\\\")\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d51d6e807121c3081f\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"variant overwritten\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"variantOverwritten\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, true), b64d(\\\\\\\\\\\\\\\"Z3JlZW4=\\\\\\\\\\\\\\\"), undefined)\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d51d6e801769c30828\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"variant with context\\\\\\\",\\\\\\\"archived\\\\\\\":false,\\\\\\\"featureFlags\\\\\\\":[{\\\\\\\"name\\\\\\\":\\\\\\\"variantWithContext\\\\\\\"}],\\\\\\\"deploymentConfiguration\\\\\\\":{\\\\\\\"condition\\\\\\\":\\\\\\\"ifThen(and(true, isInTargetGroup(\\\\\\\\\\\\\\\"5e57a3d51d6e808ac5c30822\\\\\\\\\\\\\\\")), b64d(\\\\\\\\\\\\\\\"Ymx1ZQ==\\\\\\\\\\\\\\\"), ifThen(and(true, true), undefined, undefined))\\\\\\\"},\\\\\\\"labels\\\\\\\":[],\\\\\\\"stickinessProperty\\\\\\\":\\\\\\\"rox.distinct_id\\\\\\\"}],\\\\\\\"targetGroups\\\\\\\":[{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d11d6e80596ac307c5\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d11d6e80596ac307c5\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"and(and(and(and(and(and(and(and(and(gte(property(b64d(\\\\\\\\\\\\\\\"aW50UHJvcDI=\\\\\\\\\\\\\\\")),0),lte(property(b64d(\\\\\\\\\\\\\\\"aW50UHJvcDE=\\\\\\\\\\\\\\\")),7)),eq(property(b64d(\\\\\\\\\\\\\\\"c3RyaW5nUHJvcDI=\\\\\\\\\\\\\\\")),b64d(\\\\\\\\\\\\\\\"V29ybGQ=\\\\\\\\\\\\\\\"))),inArray(md5(concat(\\\\\\\\\\\\\\\"d17a1259a931bf52\\\\\\\\\\\\\\\",property(b64d(\\\\\\\\\\\\\\\"c3RyaW5nUHJvcDE=\\\\\\\\\\\\\\\")))),[\\\\\\\\\\\\\\\"b590dce2c130bb3ceee8183e71839f24\\\\\\\\\\\\\\\",\\\\\\\\\\\\\\\"444b4449599b19bd321ba5b85c917ca4\\\\\\\\\\\\\\\"])),semverLt(property(b64d(\\\\\\\\\\\\\\\"c212clByb3Ay\\\\\\\\\\\\\\\")),b64d(\\\\\\\\\\\\\\\"MTAwLjAuNQ==\\\\\\\\\\\\\\\"))),semverEq(property(b64d(\\\\\\\\\\\\\\\"c212clByb3Ax\\\\\\\\\\\\\\\")),b64d(\\\\\\\\\\\\\\\"OS4xMS4yMDAx\\\\\\\\\\\\\\\"))),gt(property(b64d(\\\\\\\\\\\\\\\"ZG91YmxlUHJvcDI=\\\\\\\\\\\\\\\")),0.92)),lt(property(b64d(\\\\\\\\\\\\\\\"ZG91YmxlUHJvcDE=\\\\\\\\\\\\\\\")),12.1)),eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3Ay\\\\\\\\\\\\\\\")),false)),eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3Ax\\\\\\\\\\\\\\\")),true))\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d21d6e80022bc307dd\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d21d6e80022bc307dd\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3BUYXJnZXRHcm91cEZvckRlcGVuZGVuY3k=\\\\\\\\\\\\\\\")),true)\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d51d6e808ac5c30822\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d51d6e808ac5c30822\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3BUYXJnZXRHcm91cEZvclZhcmlhbnQ=\\\\\\\\\\\\\\\")),true)\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d31d6e806ec6c307f3\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d31d6e806ec6c307f3\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3BUYXJnZXRHcm91cE9wZXJhbmQx\\\\\\\\\\\\\\\")),true)\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d31d6e805829c307f1\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d31d6e805829c307f1\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3BUYXJnZXRHcm91cE9wZXJhbmQy\\\\\\\\\\\\\\\")),true)\\\\\\\"},{\\\\\\\"_id\\\\\\\":\\\\\\\"5e57a3d11d6e8060c5c307bb\\\\\\\",\\\\\\\"name\\\\\\\":\\\\\\\"5e57a3d11d6e8060c5c307bb\\\\\\\",\\\\\\\"condition\\\\\\\":\\\\\\\"eq(property(b64d(\\\\\\\\\\\\\\\"Ym9vbFByb3BUYXJnZXRHcm91cEZvclZhcmlhbnREZXBlbmRlbmN5\\\\\\\\\\\\\\\")),true)\\\\\\\"}],\\\\\\\"remoteVariables\\\\\\\":[],\\\\\\\"application\\\\\\\":\\\\\\\"5e579ecfc45c395c43b42893\\\\\\\",\\\\\\\"platform\\\\\\\":\\\\\\\"C\\\\\\\",\\\\\\\"creation_date\\\\\\\":\\\\\\\"2021-03-02T12:25:42.139Z\\\\\\\",\\\\\\\"api_version\\\\\\\":\\\\\\\"1.9.0\\\\\\\"}\\\",\\\"signature_v0\\\":\\\"4T8O6CQqE+sGbGKuG4iQM39kms+0savkoUtGF56ZkKW6oI7Auzxji2T7OzgfmdrUCuX+r0GrkYRkorRiKlryMh02aRLNk4Xv5sD+fquIeNYJJUNIoq/Qm9XLgm+vazke5nnWGV5ZXCAh08g02L5POJoJfOND3Uknh96LehZ3u67Xqc8oaeMNPNAgvDLY2K/XhV4B8TU8NJxSDjwjhhQrCWpBhmISBz+NsU3gZI0Q0pOiO+ciphYNjQKPHvOjiTuXEtVCU1b+Hi341VR/+EEScTfT+p50zllSAXcfeN40Q6/KIyCZRHsmBsbmdfEmlAcfxMP68C8MKiJojvDT5WPTtQ==\\\",\\\"signed_date\\\":\\\"2021-03-02T12:25:42.224Z\\\"}\"}";
    ServerTestContextConfig config = {NULL, ROX_MAP("config", config_data)};
    ServerTestContext *ctx = server_test_context_create_cfg(&config);

    thread_sleep(1000);
    ck_assert_int_eq(ctx->last_error, NoError);
    ck_assert_int_eq(ctx->last_status, AppliedFromLocalStorage);
    ck_assert_int_eq(1, ctx->configurationFetchedCount);

    rox_fetch();
    ck_assert_int_eq(ctx->last_error, NoError);
    ck_assert_int_eq(ctx->last_status, AppliedFromNetwork);
    ck_assert_int_eq(2, ctx->configurationFetchedCount);
    
    server_test_context_free(ctx);
#endif
}

ROX_TEST_SUITE(
// FlagTests
        ROX_TEST_CASE(test_flag_with_default_value),
        ROX_TEST_CASE(test_flag_with_default_value_after_setup),
        ROX_TEST_CASE(test_flag_with_experiment_expression_value),
        ROX_TEST_CASE(test_flag_with_experiment_returns_undefined),
        ROX_TEST_CASE(test_flag_with_experiment_wrong_type),
        ROX_TEST_CASE(test_flag_will_use_context),
        ROX_TEST_CASE(test_flag_will_invoke_enabled_action),
        ROX_TEST_CASE(test_flag_will_invoke_disabled_action),
// RoxStringTests
        ROX_TEST_CASE(test_string_will_add_default_to_options_when_no_options),
        ROX_TEST_CASE(test_string_will_not_add_default_to_options_if_exists),
        ROX_TEST_CASE(test_string_will_add_default_to_options_if_not_exists),
        ROX_TEST_CASE(test_string_will_set_name),
        ROX_TEST_CASE(test_string_will_default_no_experiment),
        ROX_TEST_CASE(test_string_will_default_after_setup),
        ROX_TEST_CASE(test_string_with_experiment),
        ROX_TEST_CASE(test_string_with_experiment_returns_undefined),
        ROX_TEST_CASE(test_string_will_use_context),
// RoxIntTests
        ROX_TEST_CASE(test_int_will_add_default_to_options_when_no_options),
        ROX_TEST_CASE(test_int_will_not_add_default_to_options_if_exists),
        ROX_TEST_CASE(test_int_will_add_default_to_options_if_not_exists),
        ROX_TEST_CASE(test_int_will_return_default_when_no_experiment),
        ROX_TEST_CASE(test_int_will_return_default_when_no_experiment_after_setup),
        ROX_TEST_CASE(test_int_will_return_default_when_experiment_returns_undefined),
        ROX_TEST_CASE(test_int_will_return_experiment_expression_value),
        ROX_TEST_CASE(test_int_will_return_default_when_wrong_experiment_type),
        ROX_TEST_CASE(test_int_will_use_context),
// RoxDoubleTests
        ROX_TEST_CASE(test_double_will_add_default_to_options_when_no_options),
        ROX_TEST_CASE(test_double_will_not_add_default_to_options_if_exists),
        ROX_TEST_CASE(test_double_will_add_default_to_options_if_not_exists),
        ROX_TEST_CASE(test_double_will_return_default),
        ROX_TEST_CASE(test_double_will_return_default_when_no_experiment_after_setup),
        ROX_TEST_CASE(test_double_will_return_default_when_experiment_returns_undefined),
        ROX_TEST_CASE(test_double_will_return_experiment_expression_value),
        ROX_TEST_CASE(test_double_will_return_default_when_experiment_wrong_type),
        ROX_TEST_CASE(test_double_will_use_context),
// RoxE2ETests
        ROX_TEST_CASE(test_simple_flag),
        ROX_TEST_CASE(test_simple_flag_overwritten),
        ROX_TEST_CASE(test_variant),
        ROX_TEST_CASE(test_variant_overwritten),
        ROX_TEST_CASE(testing_all_custom_properties),
        ROX_TEST_CASE(testing_fetch_within_timeout),
        ROX_TEST_CASE(testing_variant_with_context),
        ROX_TEST_CASE(testing_variant_with_global_context),
        ROX_TEST_CASE(testing_variant_with_global_context_set_before),
        ROX_TEST_CASE(testing_target_groups_all_any_none),
        ROX_TEST_CASE(testing_impression_handler),
        ROX_TEST_CASE(testing_flag_dependency),
        ROX_TEST_CASE(testing_variant_dependency_with_context),
        ROX_TEST_CASE(test_will_use_local_cache)
)
