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

    RoxVariant *simpleFlag;
    RoxVariant *simpleFlagOverwritten;
    RoxVariant *flagForImpression;
    RoxVariant *flagForImpressionWithExperimentAndContext;
    RoxVariant *flagCustomProperties;
    RoxVariant *flagTargetGroupsAll;
    RoxVariant *flagTargetGroupsAny;
    RoxVariant *flagTargetGroupsNone;
    RoxVariant *variantWithContext;
    RoxVariant *variant;
    RoxVariant *variantOverwritten;
    RoxVariant *flagForDependency;
    RoxVariant *flag_colors_for_dependency;
    RoxVariant *flagDependent;
    RoxVariant *flagColorDependentWithContext;

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

    char *lastImpressionValueName;
    char *lastImpressionValue;
    RoxExperiment *lastImpressionExperiment;
    RoxDynamicValue *lastImpressionContextValue;

} ServerTextContext;

static void _test_configuration_fetched_handler(void *target, RoxConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    ServerTextContext *ctx = (ServerTextContext *) target;
    if (args->fetcher_status == AppliedFromNetwork) {
        ++ctx->configurationFetchedCount;
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
        ctx->isImpressionRaised = true;
    }

    if (ctx->lastImpressionValue) {
        free(ctx->lastImpressionValue);
    }

    if (ctx->lastImpressionValueName) {
        free(ctx->lastImpressionValueName);
    }

    if (ctx->lastImpressionExperiment) {
        experiment_free(ctx->lastImpressionExperiment);
    }

    if (ctx->lastImpressionContextValue) {
        rox_dynamic_value_free(ctx->lastImpressionContextValue);
    }

    ctx->lastImpressionValueName = mem_copy_str(value->name);
    ctx->lastImpressionValue = mem_copy_str(value->value);
    ctx->lastImpressionExperiment = experiment ? experiment_copy(experiment) : NULL;
    if (context) {
        RoxDynamicValue *val = rox_context_get(context, "var");
        ctx->lastImpressionContextValue = val ? rox_dynamic_value_create_copy(val) : NULL;
    } else {
        ctx->lastImpressionContextValue = NULL;
    }

}

static RoxDynamicValue *_test_computed_string_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->isComputedStringPropCalled = true;
    return rox_dynamic_value_create_string_copy("World");
}

static RoxDynamicValue *_test_computed_boolean_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->isComputedBooleanPropCalled = true;
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
    ctx->isComputedDoublePropCalled = true;
    return rox_dynamic_value_create_double(1.618);
}

static RoxDynamicValue *_test_computed_semver_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->isComputedSemverPropCalled = true;
    return rox_dynamic_value_create_string_copy("20.7.1969");
}

static RoxDynamicValue *_test_computed_int_property(void *target, RoxContext *context) {
    ServerTextContext *ctx = (ServerTextContext *) target;
    ctx->isComputedIntPropCalled = true;
    return rox_dynamic_value_create_int(28);
}

static ServerTextContext *_server_text_context_create() {
    ServerTextContext *ctx = calloc(1, sizeof(ServerTextContext));
    ctx->logging = logging_test_fixture_create(RoxLogLevelDebug);

    RoxOptions *options = rox_options_create();
    rox_options_set_configuration_fetched_handler(options, ctx, &_test_configuration_fetched_handler);
    rox_options_set_impression_handler(options, ctx, &_test_rox_impression_handler);
    rox_options_set_dev_mode_key(options, "37d6265f591155bb00ffb4e2");

    ctx->simpleFlag = rox_add_flag("simpleFlag", true);
    ctx->simpleFlagOverwritten = rox_add_flag("simpleFlagOverwritten", true);
    ctx->flagForImpression = rox_add_flag("flagForImpression", false);
    ctx->flagForImpressionWithExperimentAndContext = rox_add_flag("flagForImpressionWithExperimentAndContext",
                                                                  false);
    ctx->flagCustomProperties = rox_add_flag("flagCustomProperties", false);
    ctx->flagTargetGroupsAll = rox_add_flag("flagTargetGroupsAll", false);
    ctx->flagTargetGroupsAny = rox_add_flag("flagTargetGroupsAny", false);
    ctx->flagTargetGroupsNone = rox_add_flag("flagTargetGroupsNone", false);
    ctx->variantWithContext = rox_add_variant("variantWithContext", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variant = rox_add_variant("variant", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->variantOverwritten = rox_add_variant("variantOverwritten", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ctx->flagForDependency = rox_add_flag("flagForDependency", false);
    ctx->flag_colors_for_dependency = rox_add_variant("flagColorsForDependency", "White",
                                                      ROX_LIST_COPY_STR("White", "Blue", "Green", "Yellow"));
    ctx->flagDependent = rox_add_flag("flagDependent", false);
    ctx->flagColorDependentWithContext = rox_add_variant("flagColorDependentWithContext", "White",
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
            &ctx->targetGroup1,
            &_test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupOperand2",
            &ctx->targetGroup2,
            &_test_computed_boolean_property_using_value);

    rox_set_custom_computed_boolean_property(
            "boolPropTargetGroupForDependency",
            &ctx->isPropForTargetGroupForDependency,
            &_test_computed_boolean_property_using_value);

    rox_setup("5e579ecfc45c395c43b42893", options);

    return ctx;
}

static void _server_text_context_free(ServerTextContext *ctx) {
    assert(ctx);
    rox_shutdown();
    logging_test_fixture_free(ctx->logging);
    if (ctx->lastImpressionValue) {
        free(ctx->lastImpressionValue);
    }
    if (ctx->lastImpressionValueName) {
        free(ctx->lastImpressionValueName);
    }
    if (ctx->lastImpressionExperiment) {
        experiment_free(ctx->lastImpressionExperiment);
    }
    if (ctx->lastImpressionContextValue) {
        rox_dynamic_value_free(ctx->lastImpressionContextValue);
    }
    free(ctx);
}

START_TEST (test_simple_flag) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(rox_flag_is_enabled(ctx->simpleFlag));
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (test_simple_flag_overwritten) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(!rox_flag_is_enabled(ctx->simpleFlagOverwritten));
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
    rox_check_and_free(rox_variant_get_value_or_default(ctx->variantOverwritten), "green");
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_all_custom_properties) {
    ServerTextContext *ctx = _server_text_context_create();
    ck_assert(rox_flag_is_enabled(ctx->flagCustomProperties));
    ck_assert(ctx->isComputedBooleanPropCalled);
    ck_assert(ctx->isComputedDoublePropCalled);
    ck_assert(ctx->isComputedIntPropCalled);
    ck_assert(ctx->isComputedSemverPropCalled);
    ck_assert(ctx->isComputedStringPropCalled);
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_fetch_within_timeout) {
    ServerTextContext *ctx = _server_text_context_create();
    int number_of_config_fetches = ctx->configurationFetchedCount;
    rox_fetch(); // TODO: do a timeout 5000
    ck_assert_int_gt(ctx->configurationFetchedCount, number_of_config_fetches);
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_variant_with_context) {
    ServerTextContext *ctx = _server_text_context_create();
    RoxContext *some_positive_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(true)));
    RoxContext *some_negative_context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("isDuckAndCover"), rox_dynamic_value_create_boolean(false)));
    rox_check_and_free(rox_variant_get_value_or_default(ctx->variantWithContext), "red");
    rox_check_and_free(rox_variant_get_value_or_default_ctx(ctx->variantWithContext, some_positive_context), "blue");
    rox_check_and_free(rox_variant_get_value_or_default_ctx(ctx->variantWithContext, some_negative_context), "red");
    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_target_groups_all_any_none) {
    ServerTextContext *ctx = _server_text_context_create();

    ctx->targetGroup1 = ctx->targetGroup2 = true;
    ck_assert(rox_flag_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(rox_flag_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(!rox_flag_is_enabled(ctx->flagTargetGroupsNone));

    ctx->targetGroup1 = false;
    ck_assert(!rox_flag_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(rox_flag_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(!rox_flag_is_enabled(ctx->flagTargetGroupsNone));

    ctx->targetGroup2 = false;
    ck_assert(!rox_flag_is_enabled(ctx->flagTargetGroupsAll));
    ck_assert(!rox_flag_is_enabled(ctx->flagTargetGroupsAny));
    ck_assert(rox_flag_is_enabled(ctx->flagTargetGroupsNone));

    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_impression_handler) {
    ServerTextContext *ctx = _server_text_context_create();

    rox_flag_is_enabled(ctx->flagForImpression);
    ck_assert(ctx->isImpressionRaised);
    ctx->isImpressionRaised = false;

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(ROX_COPY("var"), rox_dynamic_value_create_string_copy("val")));
    bool flag_impression_value = rox_flag_is_enabled_ctx(ctx->flagForImpressionWithExperimentAndContext, context);
    ck_assert_ptr_nonnull(ctx->lastImpressionValue);
    ck_assert_str_eq("true", ctx->lastImpressionValue);
    ck_assert(flag_impression_value);
    ck_assert_str_eq("flagForImpressionWithExperimentAndContext", ctx->lastImpressionValueName);

    ck_assert_ptr_nonnull(ctx->lastImpressionExperiment);
    ck_assert_str_eq("5e57a3d31d6e807bf3c307ee", ctx->lastImpressionExperiment->identifier);
    ck_assert_str_eq("flag for impression with experiment and context", ctx->lastImpressionExperiment->name);

    ck_assert_ptr_nonnull(ctx->lastImpressionContextValue);
    ck_assert(rox_dynamic_value_is_string(ctx->lastImpressionContextValue));
    ck_assert_str_eq(rox_dynamic_value_get_string(ctx->lastImpressionContextValue), "val");

    _server_text_context_free(ctx);
}

END_TEST

START_TEST (testing_flag_dependency) {
    ServerTextContext *ctx = _server_text_context_create();

    ctx->isPropForTargetGroupForDependency = true;
    ck_assert(rox_flag_is_enabled(ctx->flagForDependency));
    ck_assert(!rox_flag_is_enabled(ctx->flagDependent));

    ctx->isPropForTargetGroupForDependency = false;
    ck_assert(rox_flag_is_enabled(ctx->flagDependent));
    ck_assert(!rox_flag_is_enabled(ctx->flagForDependency));

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
            rox_variant_get_value_or_default(ctx->flagColorDependentWithContext),
            "White");

    rox_check_and_free(
            rox_variant_get_value_or_default_ctx(ctx->flagColorDependentWithContext, some_negative_context),
            "White");

    rox_check_and_free(
            rox_variant_get_value_or_default_ctx(ctx->flagColorDependentWithContext, some_positive_context),
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
