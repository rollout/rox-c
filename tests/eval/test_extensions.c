#include <check.h>
#include <assert.h>
#include "roxtests.h"
#include "eval/parser.h"
#include "eval/extensions.h"
#include "collections.h"

typedef struct ImpressionArgs {
    char *name;
    char *value;
} ImpressionArgs;

typedef struct ParserExtensionsTestContext {
    Parser *parser;
    TargetGroupRepository *target_groups_repository;
    FlagRepository *flag_repository;
    ExperimentRepository *experiment_repository;
    CustomPropertyRepository *custom_property_repository;
    DynamicProperties *dynamic_properties;
    ImpressionInvoker *impression_invoker;
    RoxList *impressions;
    const char *property_context_key;
} ParserExtensionsTestContext;

static RoxDynamicValue *parser_extensions_custom_property_generator(void *target, RoxContext *context) {
    ParserExtensionsTestContext *test_context = (ParserExtensionsTestContext *) target;
    assert(test_context);
    assert(test_context->property_context_key);
    return rox_context_get(context, test_context->property_context_key);
}

static void parser_extensions_impression_handler(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context) {
    ParserExtensionsTestContext *test_context = (ParserExtensionsTestContext *) target;
    ImpressionArgs *args = calloc(1, sizeof(ImpressionArgs));
    args->name = mem_copy_str(value->name);
    args->value = value->value ? mem_copy_str(value->value) : NULL;
    rox_list_add(test_context->impressions, args);
}

static ParserExtensionsTestContext *parser_extensions_test_context_create() {
    ParserExtensionsTestContext *context = calloc(1, sizeof(ParserExtensionsTestContext));
    context->parser = parser_create();
    context->target_groups_repository = target_group_repository_create();
    context->flag_repository = flag_repository_create();
    context->experiment_repository = experiment_repository_create();
    context->custom_property_repository = custom_property_repository_create();
    context->dynamic_properties = dynamic_properties_create();
    context->impression_invoker = impression_invoker_create();
    context->impressions = rox_list_create();
    impression_invoker_register(context->impression_invoker, context,
                                &parser_extensions_impression_handler);
    parser_add_experiments_extensions(context->parser,
                                      context->target_groups_repository,
                                      context->flag_repository,
                                      context->experiment_repository);
    parser_add_properties_extensions(context->parser,
                                     context->custom_property_repository,
                                     context->dynamic_properties);
    return context;
}

static void impression_arg_free(ImpressionArgs *args) {
    assert(args);
    free(args->name);
    if (args->value) {
        free(args->value);
    }
    free(args);
}

static void parser_extensions_test_context_free(ParserExtensionsTestContext *context) {
    assert(context);
    target_group_repository_free(context->target_groups_repository);
    flag_repository_free(context->flag_repository);
    experiment_repository_free(context->experiment_repository);
    custom_property_repository_free(context->custom_property_repository);
    dynamic_properties_free(context->dynamic_properties);
    parser_free(context->parser);
    rox_list_free_cb(context->impressions, &impression_arg_free);
    impression_invoker_free(context->impression_invoker);
    free(context);
}

START_TEST (test_custom_property_with_simple_value) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    EvaluationResult *result = parser_evaluate_expression(context->parser, "isInTargetGroup(\"targetGroup1\")", NULL);
    ck_assert(!*result_get_boolean(result));
    result_free(result);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_is_in_percentage_range) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "isInPercentageRange(0, 0.5, \"device2.seed2\")", NULL);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_not_is_in_percentage_range) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "isInPercentageRange(0.5, 1, \"device2.seed2\")", NULL);
    ck_assert(!*result_get_boolean(result));
    result_free(result);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_get_bucket) {
    double result = experiment_extensions_get_bucket("device2.seed2");
    ck_assert_double_eq(result, 0.18721251450181298);
}

END_TEST

START_TEST (test_flag_value_no_flag_no_experiment) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "flagValue(\"f1\")", NULL);
    ck_assert_str_eq(result_get_string(result), "false");
    result_free(result);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_value_no_flag_evaluate_experiment) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    experiment_repository_set_experiments(
            context->experiment_repository, ROX_LIST(
            experiment_model_create("id", "name", "\"op2\"", false, ROX_LIST_COPY_STR("f1"),
                                    ROX_EMPTY_SET, "stam")));

    EvaluationResult *result = parser_evaluate_expression(context->parser, "flagValue(\"f1\")", NULL);
    ck_assert_str_eq(result_get_string(result), "op2");
    result_free(result);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_value_flag_evaluation_default) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxStringBase *variant = variant_create_string("op1", ROX_LIST_COPY_STR("op2"));
    flag_repository_add_flag(context->flag_repository, variant, "f1");

    EvaluationResult *result = parser_evaluate_expression(context->parser, "flagValue(\"f1\")", NULL);
    ck_assert_str_eq(result_get_string(result), "op1");
    result_free(result);

    parser_extensions_test_context_free(context);
}

END_TEST

static char *variant_get_string_or_default(RoxStringBase *variant, RoxContext *context) {
    const char *default_value = variant_get_default_value(variant);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    char *result = variant_get_string(variant, default_value, eval_context);
    eval_context_free(eval_context);
    return result;
}

START_TEST (test_flag_dependency_value) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxStringBase *flag = variant_create_flag();
    flag_repository_add_flag(context->flag_repository, flag, "f1");

    RoxStringBase *v = variant_create_string("blue", ROX_LIST_COPY_STR("red", "green"));
    variant_set_for_evaluation(v, context->parser, NULL, NULL);
    variant_set_condition(v, "ifThen(eq(\"true\", flagValue(\"f1\")), \"red\", \"green\")");
    flag_repository_add_flag(context->flag_repository, v, "v1");

    char *value = variant_get_string_or_default(v, NULL);
    ck_assert_str_eq("green", value);
    free(value);

    parser_extensions_test_context_free(context);
}

END_TEST


START_TEST (test_flag_dependency_impression_handler) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxStringBase *flag = variant_create_flag();
    flag_repository_add_flag(context->flag_repository, flag, "f1");
    variant_set_for_evaluation(flag, context->parser, NULL, context->impression_invoker);

    RoxStringBase *v = variant_create_string("blue", ROX_LIST_COPY_STR("red", "green"));
    variant_set_for_evaluation(v, context->parser, NULL, context->impression_invoker);
    variant_set_condition(v, "ifThen(eq(\"true\", flagValue(\"f1\")), \"red\", \"green\")");
    flag_repository_add_flag(context->flag_repository, v, "v1");

    char *value = variant_get_string_or_default(v, NULL);
    ck_assert_str_eq("green", value);

    ck_assert_int_eq(rox_list_size(context->impressions), 2);

    ImpressionArgs *args;
    rox_list_get_at(context->impressions, 0, (void **) &args);
    ck_assert_str_eq(args->name, "f1");
    ck_assert_str_eq(args->value, "false");

    rox_list_get_at(context->impressions, 1, (void **) &args);
    ck_assert_str_eq(args->name, "v1");
    ck_assert_str_eq(args->value, "green");

    free(value);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency2_levels_bottom_not_exists) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxStringBase *flag = variant_create_flag();
    flag_repository_add_flag(context->flag_repository, flag, "f1");
    variant_set_for_evaluation(flag, context->parser, NULL, NULL);
    variant_set_condition(flag, "flagValue(\"someFlag\")");

    RoxStringBase *v = variant_create_string("blue", ROX_LIST_COPY_STR("red", "green"));
    variant_set_for_evaluation(v, context->parser, NULL, NULL);
    variant_set_condition(v, "ifThen(eq(\"true\", flagValue(\"f1\")), \"red\", \"green\")");
    flag_repository_add_flag(context->flag_repository, v, "v1");

    char *value = variant_get_string_or_default(v, NULL);
    ck_assert_str_eq("green", value);
    free(value);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency_unexisting_flag_but_existing_experiment) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxList * experiment_models = ROX_LIST(
            experiment_model_create("exp1id", "exp1name", "ifThen(true, \"true\", \"false\")", false,
                                    ROX_LIST_COPY_STR("someFlag"), ROX_EMPTY_SET, "stam"),
            experiment_model_create("exp2id", "exp2name",
                                    "ifThen(eq(\"true\", flagValue(\"someFlag\")), \"blue\", \"green\")", false,
                                    ROX_LIST_COPY_STR("colorVar"), ROX_EMPTY_SET, "stam"));

    FlagSetter *flag_setter = flag_setter_create(context->flag_repository, context->parser,
                                                 context->experiment_repository, NULL);
    experiment_repository_set_experiments(context->experiment_repository, experiment_models);
    flag_setter_set_experiments(flag_setter);

    RoxStringBase *color_var = variant_create_string("red", ROX_LIST_COPY_STR("red", "green", "blue"));
    variant_set_for_evaluation(color_var, context->parser, NULL, NULL);
    flag_repository_add_flag(context->flag_repository, color_var, "colorVar");

    char *result = variant_get_string_or_default(color_var, NULL);
    ck_assert_str_eq("blue", result);
    free(result);

    flag_setter_free(flag_setter);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency_unexisting_flag_and_experiment_undefined) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxList * experiment_models = ROX_LIST(
            experiment_model_create("exp1id", "exp1name", "undefined", false, ROX_LIST_COPY_STR("someFlag"),
                                    ROX_EMPTY_SET, "stam"),
            experiment_model_create("exp2id", "exp2name",
                                    "ifThen(eq(\"true\", flagValue(\"someFlag\")), \"blue\", \"green\")", false,
                                    ROX_LIST_COPY_STR("colorVar"), ROX_EMPTY_SET, "stam"));

    FlagSetter *flag_setter = flag_setter_create(context->flag_repository, context->parser,
                                                 context->experiment_repository, NULL);
    experiment_repository_set_experiments(context->experiment_repository, experiment_models);
    flag_setter_set_experiments(flag_setter);

    RoxStringBase *color_var = variant_create_string("red", ROX_LIST_COPY_STR("red", "green", "blue"));
    variant_set_for_evaluation(color_var, context->parser, NULL, NULL);
    flag_repository_add_flag(context->flag_repository, color_var, "colorVar");

    char *result = variant_get_string_or_default(color_var, NULL);
    ck_assert_str_eq("green", result);
    free(result);

    flag_setter_free(flag_setter);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency_with_context) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "isPropOn";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create("prop",
                                   &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
                                   context, &parser_extensions_custom_property_generator));

    RoxStringBase *flag1 = variant_create_flag();
    variant_set_for_evaluation(flag1, context->parser, NULL, NULL);
    variant_set_condition(flag1, "property(\"prop\")");
    flag_repository_add_flag(context->flag_repository, flag1, "flag1");

    RoxStringBase *flag2 = variant_create_flag();
    variant_set_for_evaluation(flag2, context->parser, NULL, NULL);
    variant_set_condition(flag2, "flagValue(\"flag1\")");
    flag_repository_add_flag(context->flag_repository, flag2, "flag2");

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("isPropOn"),
                                                          rox_dynamic_value_create_boolean(true)));

    char *flag_value = variant_get_string_or_default(flag2, ctx);
    ck_assert_str_eq(flag_value, "true");
    free(flag_value);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency_with_context_used_on_experiment_with_no_flag) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "isPropOn";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create("prop",
                                   &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
                                   context, &parser_extensions_custom_property_generator));

    RoxStringBase *flag3 = variant_create_flag();
    variant_set_for_evaluation(flag3, context->parser, NULL, NULL);
    variant_set_condition(flag3, "flagValue(\"flag2\")");
    flag_repository_add_flag(context->flag_repository, flag3, "flag3");

    RoxList * experiment_models = ROX_LIST(
            experiment_model_create("exp1id", "exp1name", "property(\"prop\")", false, ROX_LIST_COPY_STR("flag2"),
                                    ROX_EMPTY_SET, "stam"));

    experiment_repository_set_experiments(context->experiment_repository, experiment_models);

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("isPropOn"),
                                                          rox_dynamic_value_create_boolean(true)));

    char *flag_value = variant_get_string_or_default(flag3, ctx);
    ck_assert_str_eq(flag_value, "true");
    free(flag_value);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_flag_dependency_with_context2_level_mid_level_no_flag_eval_experiment) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "isPropOn";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create("prop",
                                   &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
                                   context, &parser_extensions_custom_property_generator));

    RoxStringBase *flag1 = variant_create_flag();
    variant_set_for_evaluation(flag1, context->parser, NULL, NULL);
    variant_set_condition(flag1, "property(\"prop\")");
    flag_repository_add_flag(context->flag_repository, flag1, "flag1");

    RoxStringBase *flag3 = variant_create_flag();
    variant_set_for_evaluation(flag3, context->parser, NULL, NULL);
    variant_set_condition(flag3, "flagValue(\"flag2\")");
    flag_repository_add_flag(context->flag_repository, flag3, "flag3");

    RoxList * experiment_models = ROX_LIST(
            experiment_model_create("exp1id", "exp1name", "flagValue(\"flag1\")", false, ROX_LIST_COPY_STR("flag2"),
                                    ROX_EMPTY_SET, "stam"));

    experiment_repository_set_experiments(context->experiment_repository, experiment_models);

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("isPropOn"),
                                                          rox_dynamic_value_create_boolean(true)));

    char *flag_value = variant_get_string_or_default(flag3, ctx);
    ck_assert_str_eq(flag_value, "true");
    free(flag_value);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_string) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create_using_value(
                    "testKey", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    rox_dynamic_value_create_string_copy("test")));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"test\", property(\"testKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_int) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create_using_value(
                    "testKey", &ROX_CUSTOM_PROPERTY_TYPE_INT,
                    rox_dynamic_value_create_int(3)));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(3, property(\"testKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_double) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create_using_value(
                    "testKey", &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                    rox_dynamic_value_create_double(3.3)));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(3.3, property(\"testKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_with_context_string) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "ContextTestKey";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "CustomPropertyTestKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    context, &parser_extensions_custom_property_generator));

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("ContextTestKey"),
                                                          rox_dynamic_value_create_string_copy("test")));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"test\", property(\"CustomPropertyTestKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_with_context_int) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "ContextTestKey";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "CustomPropertyTestKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_INT,
                    context, &parser_extensions_custom_property_generator));

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("ContextTestKey"),
                                                          rox_dynamic_value_create_int(3)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(3, property(\"CustomPropertyTestKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_with_context_int_with_string) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "ContextTestKey";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "CustomPropertyTestKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_INT,
                    context, &parser_extensions_custom_property_generator));

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("ContextTestKey"),
                                                          rox_dynamic_value_create_int(3)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"3\", property(\"CustomPropertyTestKey\"))", eval_context);
    ck_assert(!*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_roxx_properties_extensions_with_context_int_not_equal) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    context->property_context_key = "ContextTestKey";

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "CustomPropertyTestKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_INT,
                    context, &parser_extensions_custom_property_generator));

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("ContextTestKey"),
                                                          rox_dynamic_value_create_int(3)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(4, property(\"CustomPropertyTestKey\"))", eval_context);
    ck_assert(!*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_unknown_property) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create_using_value(
                    "testKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    rox_dynamic_value_create_string_copy("test")));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"test\", property(\"testKey1\"))", eval_context);
    ck_assert(!*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

END_TEST

static RoxDynamicValue *test_get_null_from_context(void *target, RoxContext *context) {
    return NULL;
}

START_TEST (test_null_property) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "testKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    NULL, &test_get_null_from_context));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(undefined, property(\"testKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

static RoxDynamicValue *test_get_null_value_from_context(void *target, RoxContext *context) {
    return rox_dynamic_value_create_null();
}

START_TEST (test_null_value_property) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    custom_property_repository_add_custom_property(
            context->custom_property_repository,
            custom_property_create(
                    "testKey",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    NULL, &test_get_null_value_from_context));

    EvaluationContext *eval_context = eval_context_create(NULL, NULL);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(undefined, property(\"testKey\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_default_dynamic_rule) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("testKeyRule"),
                                                          rox_dynamic_value_create_string_copy("test")));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"test\", property(\"testKeyRule\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

static RoxDynamicValue *test_inc_value(const char *property_name, void *target, RoxContext *context) {
    RoxDynamicValue *value = rox_context_get(context, property_name);
    int i = rox_dynamic_value_get_int(value);
    rox_dynamic_value_free(value);
    return rox_dynamic_value_create_int(i + 1);
}

START_TEST (test_custom_dynamic_rule) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();
    dynamic_properties_set_rule(context->dynamic_properties, NULL, &test_inc_value);

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("testKeyRule"),
                                                          rox_dynamic_value_create_int(5)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(6, property(\"testKeyRule\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_dynamic_rule_returns_null) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxContext *ctx = rox_context_create_from_map(ROX_MAP(
                                                          mem_copy_str("testKeyRule"),
                                                          NULL));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(undefined, property(\"testKeyRule\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_dynamic_rule_returns_supported_type) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxContext *ctx = rox_context_create_from_map(
            ROX_MAP(
                    mem_copy_str("testKeyRule"), rox_dynamic_value_create_string_copy("test1"),
                    mem_copy_str("testKeyRule2"), rox_dynamic_value_create_boolean(true),
                    mem_copy_str("testKeyRule3"), rox_dynamic_value_create_double(3.9999),
                    mem_copy_str("testKeyRule4"), rox_dynamic_value_create_int(100)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);

    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(\"test1\", property(\"testKeyRule\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);

    result = parser_evaluate_expression(context->parser, "eq(true, property(\"testKeyRule2\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);

    result = parser_evaluate_expression(context->parser, "eq(3.9999, property(\"testKeyRule3\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);

    result = parser_evaluate_expression(context->parser, "eq(100, property(\"testKeyRule4\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);

    eval_context_free(eval_context);
    rox_context_free(ctx);
    parser_extensions_test_context_free(context);
}

END_TEST

START_TEST (test_dynamic_rule_return_unsupported_type) {
    ParserExtensionsTestContext *context = parser_extensions_test_context_create();

    RoxMap * map = rox_map_create();
    RoxContext *ctx = rox_context_create_from_map(
            ROX_MAP(
                    mem_copy_str("testKeyRule"),
                    rox_dynamic_value_create_map(map)));

    EvaluationContext *eval_context = eval_context_create(NULL, ctx);
    EvaluationResult *result = parser_evaluate_expression(
            context->parser, "eq(undefined, property(\"testKeyRule\"))", eval_context);
    ck_assert(*result_get_boolean(result));
    result_free(result);
    eval_context_free(eval_context);
    rox_context_free(ctx);

    parser_extensions_test_context_free(context);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_custom_property_with_simple_value),
        ROX_TEST_CASE(test_is_in_percentage_range),
        ROX_TEST_CASE(test_not_is_in_percentage_range),
        ROX_TEST_CASE(test_get_bucket),
        ROX_TEST_CASE(test_flag_value_no_flag_no_experiment),
        ROX_TEST_CASE(test_flag_value_no_flag_evaluate_experiment),
        ROX_TEST_CASE(test_flag_value_flag_evaluation_default),
        ROX_TEST_CASE(test_flag_dependency_value),
        ROX_TEST_CASE(test_flag_dependency_impression_handler),
        ROX_TEST_CASE(test_flag_dependency2_levels_bottom_not_exists),
        ROX_TEST_CASE(test_flag_dependency_unexisting_flag_but_existing_experiment),
        ROX_TEST_CASE(test_flag_dependency_unexisting_flag_and_experiment_undefined),
        ROX_TEST_CASE(test_flag_dependency_with_context),
        ROX_TEST_CASE(test_flag_dependency_with_context_used_on_experiment_with_no_flag),
        ROX_TEST_CASE(test_flag_dependency_with_context2_level_mid_level_no_flag_eval_experiment),
        ROX_TEST_CASE(test_roxx_properties_extensions_string),
        ROX_TEST_CASE(test_roxx_properties_extensions_int),
        ROX_TEST_CASE(test_roxx_properties_extensions_double),
        ROX_TEST_CASE(test_roxx_properties_extensions_with_context_string),
        ROX_TEST_CASE(test_roxx_properties_extensions_with_context_int),
        ROX_TEST_CASE(test_roxx_properties_extensions_with_context_int_with_string),
        ROX_TEST_CASE(test_roxx_properties_extensions_with_context_int_not_equal),
        ROX_TEST_CASE(test_unknown_property),
        ROX_TEST_CASE(test_null_property),
        ROX_TEST_CASE(test_null_value_property),
        ROX_TEST_CASE(test_default_dynamic_rule),
        ROX_TEST_CASE(test_custom_dynamic_rule),
        ROX_TEST_CASE(test_dynamic_rule_returns_null),
        ROX_TEST_CASE(test_dynamic_rule_returns_supported_type),
        ROX_TEST_CASE(test_dynamic_rule_return_unsupported_type)
)
