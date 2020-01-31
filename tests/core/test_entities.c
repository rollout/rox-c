#include <check.h>
#include <core/repositories.h>
#include "roxtests.h"
#include "core/entities.h"
#include "util.h"

//
// VariantTests
//

void check_variant_value_eq(Variant *variant, const char *expected_value) {
    char *string = variant_get_value_or_default(variant, NULL);
    ck_assert_str_eq(string, expected_value);
    free(string);
}

START_TEST (test_will_not_add_default_to_options_if_exists) {
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("1", "2", "3"));
    ck_assert_int_eq(list_size(variant->options), 3);
    variant_free(variant);
}

END_TEST

START_TEST (test_will_add_default_to_options_if_not_exists) {
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_int_eq(list_size(variant->options), 3);
    ck_assert(str_in_list("1", variant->options));
    variant_free(variant);
}

END_TEST

START_TEST (test_will_set_name) {
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ck_assert_ptr_null(variant->name);
    variant_set_name(variant, "bop");
    ck_assert_str_eq(variant->name, "bop");
    variant_free(variant);
}

END_TEST

START_TEST (test_will_return_default_value_when_no_parser_or_condition) {
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    check_variant_value_eq(variant, "1");

    Parser *parser = parser_create();
    variant_set_for_evaluation(variant, parser, NULL, NULL);
    check_variant_value_eq(variant, "1");
    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "123", false,
            ROX_LIST_COPY_STR("1"), ROX_EMPTY_HASH_SET,
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
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "\"xxx\"", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_HASH_SET, "stam");
    variant_set_for_evaluation(variant, parser, experiment, NULL);
    check_variant_value_eq(variant, "xxx");
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
}

END_TEST

START_TEST (test_will_return_value_when_on_evaluation) {
    Parser *parser = parser_create();
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ExperimentModel *experiment = experiment_model_create("id", "name", "2", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_HASH_SET, "stam");
    variant_set_for_evaluation(variant, parser, experiment, NULL);
    check_variant_value_eq(variant, "2");
    variant_free(variant);
    experiment_model_free(experiment);
    parser_free(parser);
}

END_TEST

static bool test_impression_raised = false;

void test_impression_handler(void *target, ReportingValue *value, Experiment *experiment, Context *context) {
    test_impression_raised = true;
}

START_TEST (test_will_raise_impression) {
    Parser *parser = parser_create();
    Variant *variant = variant_create("1", ROX_LIST_COPY_STR("2", "3"));
    ImpressionInvoker *imp_invoker = impression_invoker_create();
    impression_invoker_register(imp_invoker, NULL, &test_impression_handler);
    ExperimentModel *experiment = experiment_model_create("id", "name", "2", false, ROX_LIST_COPY_STR("1"),
                                                          ROX_EMPTY_HASH_SET, "stam");
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
    Variant *flag = variant_create_flag();
    ck_assert(!flag_is_enabled(flag, NULL));
    variant_free(flag);
}

END_TEST

START_TEST (test_flag_with_default_value) {
    Variant *flag = variant_create_flag_with_default(true);
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
    Variant *flag = variant_create_flag_with_default(true);
    flag_enabled_do(flag, NULL, &test_flag_action);
    ck_assert(test_flag_action_called);
    variant_free(flag);
}

END_TEST

START_TEST (test_will_invoke_disabled_action) {
    test_flag_action_called = false;
    Variant *flag = variant_create_flag();
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
                                    ROX_EMPTY_HASH_SET, "stam")
    ));
    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    Variant *flag = flag_repository_get_flag(flag_repo, "f1");
    ck_assert_str_eq(flag->condition, "1");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_str_eq(flag->experiment->id, "33");

    flag_setter_free(flag_setter);
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
                                    ROX_EMPTY_HASH_SET, "stam")
    ));

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    Variant *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(flag->condition, "");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_ptr_null(flag->experiment);

    flag_setter_free(flag_setter);
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

    Variant *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(flag->condition, "");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_ptr_null(flag->experiment);

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("id1", "1", "con", false, ROX_LIST_COPY_STR("f2"),
                                    ROX_EMPTY_HASH_SET, "stam")
    ));

    flag_setter_set_experiments(flag_setter);

    flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(flag->condition, "con");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_ptr_nonnull(flag->experiment);
    ck_assert_str_eq(flag->experiment->id, "id1");

    flag_setter_free(flag_setter);
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
                                    ROX_EMPTY_HASH_SET, "stam")
    ));

    flag_setter_set_experiments(flag_setter);

    Variant *flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(flag->condition, "con1");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_ptr_nonnull(flag->experiment);
    ck_assert_str_eq(flag->experiment->id, "id1");


    experiment_repository_set_experiments(exp_repo, ROX_EMPTY_LIST);

    flag_setter_set_experiments(flag_setter);

    flag = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(flag->condition, "");
    ck_assert_ptr_eq(flag->parser, parser);
    ck_assert_ptr_eq(flag->impression_invoker, impression_invoker);
    ck_assert_ptr_null(flag->experiment);

    flag_setter_free(flag_setter);
}

END_TEST

START_TEST (test_will_set_data_for_added_flag) {

    FlagRepository *flag_repo = flag_repository_create();
    ExperimentRepository *exp_repo = experiment_repository_create();
    Parser *parser = parser_create();
    ImpressionInvoker *impression_invoker = impression_invoker_create();

    experiment_repository_set_experiments(exp_repo, ROX_LIST(
            experiment_model_create("1", "1", "1", false, ROX_LIST_COPY_STR("f1"),
                                    ROX_EMPTY_HASH_SET, "stam")
    ));

    FlagSetter *flag_setter = flag_setter_create(flag_repo, parser, exp_repo, impression_invoker);
    flag_setter_set_experiments(flag_setter);

    flag_repository_add_flag(flag_repo, variant_create_flag(), "f1");
    flag_repository_add_flag(flag_repo, variant_create_flag(), "f2");

    Variant *f1 = flag_repository_get_flag(flag_repo, "f1");
    ck_assert_str_eq(f1->condition, "1");
    ck_assert_ptr_eq(f1->parser, parser);
    ck_assert_ptr_eq(f1->impression_invoker, impression_invoker);
    ck_assert_ptr_nonnull(f1->experiment);
    ck_assert_str_eq(f1->experiment->id, "1");

    Variant *f2 = flag_repository_get_flag(flag_repo, "f2");
    ck_assert_str_eq(f2->condition, "");
    ck_assert_ptr_eq(f2->parser, parser);
    ck_assert_ptr_eq(f2->impression_invoker, impression_invoker);
    ck_assert_ptr_null(f2->experiment);

    flag_setter_free(flag_setter);
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
//// FlagTests
        ROX_TEST_CASE(test_flag_without_default_value),
        ROX_TEST_CASE(test_flag_with_default_value),
        ROX_TEST_CASE(test_will_invoke_enabled_action),
        ROX_TEST_CASE(test_will_invoke_disabled_action),
//// FlagSetterTests
        ROX_TEST_CASE(test_will_set_flag_data),
        ROX_TEST_CASE(test_will_not_set_for_other_flag),
        ROX_TEST_CASE(test_will_set_experiment_for_flag_and_will_remove_it),
        ROX_TEST_CASE(test_will_set_flag_without_experiment_and_then_add_experiment),
        ROX_TEST_CASE(test_will_set_data_for_added_flag)
)