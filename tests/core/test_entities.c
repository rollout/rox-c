#include <check.h>
#include <core/repositories.h>
#include "roxtests.h"
#include "core/entities.h"
#include "util.h"

//
// VariantTests
//

void check_variant_value_eq(RoxVariant *variant, const char *expected_value) {
    char *string = variant_get_value_or_default(variant, NULL);
    ck_assert_str_eq(string, expected_value);
    free(string);
}

START_TEST (test_will_not_add_default_to_options_if_exists) {
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("1", "2", "3"));
    ck_assert_int_eq(list_size(variant_get_options(variant)), 3);
    variant_free(variant);
}

END_TEST

START_TEST (test_will_add_default_to_options_if_not_exists) {
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_int_eq(list_size(variant_get_options(variant)), 3);
    ck_assert(str_in_list("1", variant_get_options(variant)));
    variant_free(variant);
}

END_TEST

START_TEST (test_will_set_name) {
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_ptr_null(variant_get_name(variant));
    variant_set_name(variant, "bop");
    ck_assert_str_eq(variant_get_name(variant), "bop");
    variant_free(variant);
}

END_TEST

START_TEST (test_will_return_default_value_when_no_parser_or_condition) {
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    check_variant_value_eq(variant, "1");

    Parser *parser = parser_create();
    variant_set_for_evaluation(variant, parser, NULL, NULL);
    check_variant_value_eq(variant, "1");
    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "123", false,
            ROX_LIST_COPY_STR("1"), ROX_EMPTY_SET,
            "stam");
    variant_set_for_evaluation(variant, NULL, experiment, NULL);
    check_variant_value_eq(variant, "1");
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_expression_value_when_result_not_in_options) {
    Parser *parser = parser_create();
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"xxx\"", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_set_for_evaluation(variant, parser, experiment, NULL);
    check_variant_value_eq(variant, "xxx");
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_return_value_when_on_evaluation) {
    Parser *parser = parser_create();
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "2", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_set_for_evaluation(variant, parser, experiment, NULL);
    check_variant_value_eq(variant, "2");
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
}

END_TEST

static bool test_impression_raised = false;

void _test_impression_handler(void *target, RoxReportingValue *value, RoxExperiment *experiment, RoxContext *context) {
    test_impression_raised = true;
}

START_TEST (test_will_raise_impression) {
    Parser *parser = parser_create();
    RoxVariant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ImpressionInvoker *imp_invoker = impression_invoker_create();
    impression_invoker_register(imp_invoker, NULL, &_test_impression_handler);
    ExperimentModel *experiment = experiment_model_create("id", "name", "2", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_SET, "stam");
    variant_set_for_evaluation(variant, parser, experiment, imp_invoker);
    check_variant_value_eq(variant, "2");
    ck_assert(test_impression_raised);
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
    impression_invoker_free(imp_invoker);
}

END_TEST

//
// FlagTests
//

START_TEST (test_flag_without_default_value) {
    RoxVariant *flag = variant_create_flag();
    ck_assert(!flag_is_enabled(flag, NULL));
    variant_free(flag);
}

END_TEST

START_TEST (test_flag_with_default_value) {
    RoxVariant *flag = variant_create_flag_with_default(true);
    ck_assert(flag_is_enabled(flag, NULL));
    variant_free(flag);
}

END_TEST

static bool test_flag_action_called = false;

void test_flag_action() {
    test_flag_action_called = true;
}

START_TEST (test_will_invoke_enabled_action) {
    test_flag_action_called = false;
    RoxVariant *flag = variant_create_flag_with_default(true);
    flag_enabled_do(flag, NULL, &test_flag_action);
    ck_assert(test_flag_action_called);
    variant_free(flag);
}

END_TEST

START_TEST (test_will_invoke_disabled_action) {
    test_flag_action_called = false;
    RoxVariant *flag = variant_create_flag();
    flag_disabled_do(flag, NULL, &test_flag_action);
    ck_assert(test_flag_action_called);
    variant_free(flag);
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

    RoxVariant *flag = flag_repository_get_flag(flag_repo, "f1");
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

    RoxVariant *flag = flag_repository_get_flag(flag_repo, "f2");
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

    RoxVariant *flag = flag_repository_get_flag(flag_repo, "f2");
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

    RoxVariant *flag = flag_repository_get_flag(flag_repo, "f2");
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

    RoxVariant *f1 = flag_repository_get_flag(flag_repo, "f1");
    ck_assert_str_eq(variant_get_condition(f1), "1");
    ck_assert_ptr_eq(variant_get_parser(f1), parser);
    ck_assert_ptr_eq(variant_get_impression_invoker(f1), impression_invoker);
    ck_assert_ptr_nonnull(variant_get_experiment(f1));
    ck_assert_str_eq(variant_get_experiment(f1)->id, "1");

    RoxVariant *f2 = flag_repository_get_flag(flag_repo, "f2");
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
// VariantTests
        ROX_TEST_CASE(test_will_not_add_default_to_options_if_exists),
        ROX_TEST_CASE(test_will_add_default_to_options_if_not_exists),
        ROX_TEST_CASE(test_will_set_name),
        ROX_TEST_CASE(test_will_return_default_value_when_no_parser_or_condition),
        ROX_TEST_CASE(test_will_expression_value_when_result_not_in_options),
        ROX_TEST_CASE(test_will_return_value_when_on_evaluation),
        ROX_TEST_CASE(test_will_raise_impression),
// FlagTests
        ROX_TEST_CASE(test_flag_without_default_value),
        ROX_TEST_CASE(test_flag_with_default_value),
        ROX_TEST_CASE(test_will_invoke_enabled_action),
        ROX_TEST_CASE(test_will_invoke_disabled_action),
// FlagSetterTests
        ROX_TEST_CASE(test_will_set_flag_data),
        ROX_TEST_CASE(test_will_not_set_for_other_flag),
        ROX_TEST_CASE(test_will_set_experiment_for_flag_and_will_remove_it),
        ROX_TEST_CASE(test_will_set_flag_without_experiment_and_then_add_experiment),
        ROX_TEST_CASE(test_will_set_data_for_added_flag)
)