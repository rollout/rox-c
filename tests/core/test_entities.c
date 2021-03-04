#include <check.h>
#include <core/repositories.h>
#include "roxtests.h"
#include "util.h"

//
// VariantTests
//

typedef struct VariantTestContext {
    Parser *parser;
    ImpressionInvoker *imp_invoker;
    bool test_impression_raised;
    char *last_impression_value;
    const char *imp_context_key;
    RoxDynamicValue *imp_context_value;
} VariantTestContext;

static void test_impression_handler(
        void *target,
        RoxReportingValue *value,
        RoxContext *context) {
    VariantTestContext *ctx = (VariantTestContext *) target;
    ctx->test_impression_raised = true;
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
    }
    if (ctx->imp_context_value) {
        rox_dynamic_value_free(ctx->imp_context_value);
        ctx->imp_context_value = NULL;
    }
    ctx->last_impression_value = mem_copy_str(value->value);
    if (ctx->imp_context_key) {
        ctx->imp_context_value = rox_context_get(context, ctx->imp_context_key);
    }
}

static VariantTestContext *variant_test_context_create() {
    VariantTestContext *ctx = calloc(1, sizeof(VariantTestContext));
    ctx->parser = parser_create();
    ctx->imp_invoker = impression_invoker_create();
    impression_invoker_register(ctx->imp_invoker, ctx, &test_impression_handler);
    return ctx;
}

static void variant_test_context_apply(VariantTestContext *ctx, RoxStringBase *variant) {
    variant_set_for_evaluation(variant, ctx->parser, NULL, ctx->imp_invoker);
}

static void variant_test_context_apply_with_experiment(
        VariantTestContext *ctx,
        RoxStringBase *variant,
        ExperimentModel *experiment) {
    variant_set_for_evaluation(variant, ctx->parser, experiment, ctx->imp_invoker);
}

static void check_no_impression(VariantTestContext *ctx) {
    ck_assert(!ctx->test_impression_raised);
}

static void check_impression(VariantTestContext *ctx, const char *value) {
    ck_assert(ctx->test_impression_raised);
    ck_assert_str_eq(value, ctx->last_impression_value);
    ctx->test_impression_raised = false;
    free(ctx->last_impression_value);
    ctx->last_impression_value = NULL;
}

static void variant_test_context_free(VariantTestContext *ctx) {
    impression_invoker_free(ctx->imp_invoker);
    parser_free(ctx->parser);
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
        ctx->last_impression_value = NULL;
    }
    if (ctx->imp_context_value) {
        rox_dynamic_value_free(ctx->imp_context_value);
        ctx->imp_context_value = NULL;
    }
    free(ctx);
}

static void check_variant_value_eq(RoxStringBase *variant, const char *expected_value) {
    const char *default_value = variant_get_default_value(variant);
    EvaluationContext *eval_context = eval_context_create(variant, NULL);
    char *string = variant_get_string(variant, default_value, eval_context);
    ck_assert_str_eq(string, expected_value);
    free(string);
    eval_context_free(eval_context);
}

static void check_variant_value_ctx_eq(RoxStringBase *variant, RoxContext *context, const char *expected_value) {
    const char *default_value = variant_get_default_value(variant);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    char *string = variant_get_string(variant, default_value, eval_context);;
    ck_assert_str_eq(string, expected_value);
    free(string);
    eval_context_free(eval_context);
}

START_TEST (test_will_add_default_to_options_when_no_options) {
    RoxStringBase *variant = variant_create_string("1", NULL);
    RoxList *list = variant_get_options(variant);
    ck_assert_int_eq(rox_list_size(list), 1);
    ck_assert(str_in_list("1", list));
    variant_free(variant);
}

END_TEST

START_TEST (test_will_not_add_default_to_options_if_exists) {
    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("1", "2", "3"));
    ck_assert_int_eq(rox_list_size(variant_get_options(variant)), 3);
    variant_free(variant);
}

END_TEST

START_TEST (test_will_add_default_to_options_if_not_exists) {
    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_int_eq(rox_list_size(variant_get_options(variant)), 3);
    ck_assert(str_in_list("1", variant_get_options(variant)));
    variant_free(variant);
}

END_TEST

START_TEST (test_will_set_name) {
    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_ptr_null(variant_get_name(variant));
    variant_set_name(variant, "bop");
    ck_assert_str_eq(variant_get_name(variant), "bop");
    variant_free(variant);
}

END_TEST

START_TEST (test_will_return_default_value_when_no_parser_or_condition) {
    VariantTestContext *ctx = variant_test_context_create();

    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    variant_set_for_evaluation(variant, NULL, NULL, ctx->imp_invoker);
    check_variant_value_eq(variant, "1");
    check_impression(ctx, "1");

    variant_set_for_evaluation(variant, ctx->parser, NULL, ctx->imp_invoker);
    check_variant_value_eq(variant, "1");
    check_impression(ctx, "1");

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "123", false,
            ROX_LIST_COPY_STR("1"), ROX_EMPTY_SET,
            "stam");
    variant_set_for_evaluation(variant, NULL, experiment, ctx->imp_invoker);
    check_variant_value_eq(variant, "1");
    check_impression(ctx, "1");

    variant_free(variant);
    experiment_model_free(experiment);
    variant_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_expression_value_when_result_not_in_options) {
    VariantTestContext *ctx = variant_test_context_create();

    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"xxx\"", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_test_context_apply_with_experiment(ctx, variant, experiment);
    check_variant_value_eq(variant, "xxx");
    check_impression(ctx, "xxx");

    variant_free(variant);
    experiment_model_free(experiment);
    variant_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_return_value_when_on_evaluation) {
    VariantTestContext *ctx = variant_test_context_create();
    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"2\"", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_set_for_evaluation(variant, ctx->parser, experiment, ctx->imp_invoker);
    check_variant_value_eq(variant, "2");
    check_impression(ctx, "2");

    variant_free(variant);
    experiment_model_free(experiment);
    variant_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_use_context) {
    VariantTestContext *ctx = variant_test_context_create();
    ctx->imp_context_key = "key";

    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"2\"", false, ROX_LIST_COPY_STR("flag"),
                                                          ROX_EMPTY_SET, "stam");
    variant_test_context_apply_with_experiment(ctx, variant, experiment);

    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("key"), rox_dynamic_value_create_int(55)));

    check_variant_value_ctx_eq(variant, context, "2");
    check_impression(ctx, "2");
    ck_assert(ctx->imp_context_value);
    ck_assert_int_eq(55, rox_dynamic_value_get_int(ctx->imp_context_value));

    rox_context_free(context);
    variant_free(variant);
    experiment_model_free(experiment);
    variant_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_raise_impression) {
    VariantTestContext *ctx = variant_test_context_create();

    RoxStringBase *variant = variant_create_string("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"2\"", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_test_context_apply_with_experiment(ctx, variant, experiment);
    check_variant_value_eq(variant, "2");
    check_impression(ctx, "2");

    variant_free(variant);
    experiment_model_free(experiment);
    variant_test_context_free(ctx);
}

END_TEST

//
// FlagTests
//

START_TEST (test_flag_without_default_value) {
    RoxStringBase *flag = variant_create_flag();
    EvaluationContext *eval_context = eval_context_create(flag, NULL);
    ck_assert(!variant_get_bool(flag, NULL, eval_context));
    variant_free(flag);
    eval_context_free(eval_context);
}

END_TEST

START_TEST (test_flag_with_default_value) {
    RoxStringBase *flag = variant_create_flag_with_default(true);
    EvaluationContext *eval_context = eval_context_create(flag, NULL);
    ck_assert(variant_get_bool(flag, NULL, eval_context));
    variant_free(flag);
    eval_context_free(eval_context);
}

END_TEST

//
// FlagSetterTests
//

START_TEST (test_will_set_flag_data) {
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    flag_repository_add_flag(flag_repo, variant_create_flag(), "f1");
    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("33", "1", "1", false, ROX_LIST_COPY_STR("f1"),
                                    ROX_EMPTY_SET, "stam")
    ));
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    RoxStringBase *flag = flag_repository_get_flag(flag_repo, "f1");
    ck_assert_str_eq(variant_get_condition(flag), "1");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_str_eq(variant_get_experiment(flag)->id, "33");

    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
}

END_TEST

START_TEST (test_will_not_set_for_other_flag) {
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    flag_repository_add_flag(flag_repo, variant_create_flag(), "f1");
    flag_repository_add_flag(flag_repo, variant_create_flag(), "f2");

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "1", "1", false, ROX_LIST_COPY_STR("f1"),
                                    ROX_EMPTY_SET, "stam")
    ));

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    RoxStringBase *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(flag), "");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_ptr_null(variant_get_experiment(flag));

    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
}

END_TEST

START_TEST (test_will_set_experiment_for_flag_and_will_remove_it) {
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_repository_add_flag(flag_repo, variant_create_flag(), "f2");
    flag_setter_set_experiments(flag_setter);

    RoxStringBase *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(flag), "");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_ptr_null(variant_get_experiment(flag));

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id1", "1", "con", false, ROX_LIST_COPY_STR("f2"),
                                    ROX_EMPTY_SET, "stam")
    ));

    flag_setter_set_experiments(flag_setter);

    flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(flag), "con");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_ptr_nonnull(variant_get_experiment(flag));
    ck_assert_str_eq(variant_get_experiment(flag)->id, "id1");

    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
}

END_TEST

START_TEST (test_will_set_flag_without_experiment_and_then_add_experiment) {
    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);

    flag_repository_add_flag(flag_repo, variant_create_flag(), "f2");
    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id1", "1", "con1", false, ROX_LIST_COPY_STR("f2"),
                                    ROX_EMPTY_SET, "stam")
    ));

    flag_setter_set_experiments(flag_setter);

    RoxStringBase *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(flag), "con1");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_ptr_nonnull(variant_get_experiment(flag));
    ck_assert_str_eq(variant_get_experiment(flag)->id, "id1");

    experiment_repository_set_experiments(exp_repo, ROX_EMPTY_LIST);

    flag_setter_set_experiments(flag_setter);

    flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(flag), "");
    ck_assert_ptr_eq(variant_get_parser(flag), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(flag), impression_invoker);
    ck_assert_ptr_null(variant_get_experiment(flag));

    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
}

END_TEST

START_TEST (test_will_set_data_for_added_flag) {

    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "1", "1", false, ROX_LIST_COPY_STR("f1"),
                                    ROX_EMPTY_SET, "stam")
    ));

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    flag_repository_add_flag(flag_repo, variant_create_flag(), "f1");
    flag_repository_add_flag(flag_repo, variant_create_flag(), "f2");

    RoxStringBase *f1 = flag_repository_get_flag(flag_repo, "f1");
    ck_assert_str_eq(variant_get_condition(f1), "1");
    ck_assert_ptr_eq(variant_get_parser(f1), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(f1), impression_invoker);
    ck_assert_ptr_nonnull(variant_get_experiment(f1));
    ck_assert_str_eq(variant_get_experiment(f1)->id, "1");

    RoxStringBase *f2 = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(variant_get_condition(f2), "");
    ck_assert_ptr_eq(variant_get_parser(f2), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(f2), impression_invoker);
    ck_assert_ptr_null(variant_get_experiment(f2));

    flag_setter_free(flag_setter);
    flag_repository_free(flag_repo);
    parser_free(parser);
    experiment_repository_free(exp_repo);
    impression_invoker_free(impression_invoker);
}

END_TEST

ROX_TEST_SUITE(
// RoxStringTests
        ROX_TEST_CASE(test_will_add_default_to_options_when_no_options),
        ROX_TEST_CASE(test_will_not_add_default_to_options_if_exists),
        ROX_TEST_CASE(test_will_add_default_to_options_if_not_exists),
        ROX_TEST_CASE(test_will_set_name),
        ROX_TEST_CASE(test_will_return_default_value_when_no_parser_or_condition),
        ROX_TEST_CASE(test_will_expression_value_when_result_not_in_options),
        ROX_TEST_CASE(test_will_return_value_when_on_evaluation),
        ROX_TEST_CASE(test_will_use_context),
        ROX_TEST_CASE(test_will_raise_impression),
// FlagTests
        ROX_TEST_CASE(test_flag_without_default_value),
        ROX_TEST_CASE(test_flag_with_default_value),
// FlagSetterTests
        ROX_TEST_CASE(test_will_set_flag_data),
        ROX_TEST_CASE(test_will_not_set_for_other_flag),
        ROX_TEST_CASE(test_will_set_experiment_for_flag_and_will_remove_it),
        ROX_TEST_CASE(test_will_set_flag_without_experiment_and_then_add_experiment),
        ROX_TEST_CASE(test_will_set_data_for_added_flag)
)