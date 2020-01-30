#include <check.h>
#include "roxtests.h"
#include "core/repositories.h"
#include "util.h"

START_TEST (test_custom_property_repo_will_return_null_when_prop_not_found) {
    CustomPropertyRepository *repo = custom_property_repository_create();
    ck_assert_ptr_null(custom_property_repository_get_custom_property(repo, "harti"));
    custom_property_repository_free(repo);
}

END_TEST

START_TEST (test_custom_property_repo_will_add_prop) {
    CustomPropertyRepository *repo = custom_property_repository_create();
    CustomProperty *cp = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                            dynamic_value_create_string_copy("123"));
    custom_property_repository_add_custom_property(repo, cp);
    CustomProperty *prop = custom_property_repository_get_custom_property(repo, "prop1");
    ck_assert_ptr_nonnull(prop);
    ck_assert_str_eq(custom_property_get_name(prop), "prop1");
    custom_property_repository_free(repo);
}

END_TEST

START_TEST (test_custom_property_repo_will_not_override_prop) {

    CustomPropertyRepository *repo = custom_property_repository_create();
    CustomProperty *cp = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                            dynamic_value_create_string_copy("123"));
    CustomProperty *cp2 = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                             dynamic_value_create_string_copy("234"));

    ck_assert(custom_property_repository_add_custom_property_if_not_exists(repo, cp));
    ck_assert(!custom_property_repository_add_custom_property_if_not_exists(repo, cp2));

    CustomProperty *prop = custom_property_repository_get_custom_property(repo, "prop1");
    ck_assert_ptr_nonnull(prop);
    rox_check_prop_str(prop, "123", NULL);

    custom_property_free(cp2);
    custom_property_repository_free(repo);
}

END_TEST

START_TEST (test_custom_property_repo_will_override_prop) {
    CustomPropertyRepository *repo = custom_property_repository_create();
    CustomProperty *cp = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                            dynamic_value_create_string_copy("123"));
    CustomProperty *cp2 = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                             dynamic_value_create_string_copy("234"));
    ck_assert(custom_property_repository_add_custom_property_if_not_exists(repo, cp));
    custom_property_repository_add_custom_property(repo, cp2);
    CustomProperty *prop = custom_property_repository_get_custom_property(repo, "prop1");
    ck_assert_ptr_nonnull(prop);
    rox_check_prop_str(prop, "234", NULL);
    custom_property_repository_free(repo);
}

END_TEST

static CustomProperty *TEST_REPO_HANDLER_PROP;

void test_property_handler(CustomProperty *property) {
    TEST_REPO_HANDLER_PROP = property;
}

START_TEST (test_custom_property_repo_will_raise_prop_added_event) {
    CustomPropertyRepository *repo = custom_property_repository_create();
    custom_property_repository_set_handler(repo, &test_property_handler);
    CustomProperty *cp = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                            dynamic_value_create_string_copy("123"));
    custom_property_repository_add_custom_property(repo, cp);
    ck_assert_ptr_eq(TEST_REPO_HANDLER_PROP, cp);
    custom_property_repository_free(repo);
}

END_TEST

START_TEST (test_experiment_repository_will_return_null_when_not_found) {
    List *exp = ROX_LIST(
            experiment_model_create("1", "1", "1", false, ROX_LIST(ROX_COPY("a")), ROX_EMPTY_HASH_SET, "stam"));
    ExperimentRepository *repo = experiment_repository_create();
    experiment_repository_set_experiments(repo, exp);
    ExperimentModel *experiment = experiment_repository_get_experiment_by_flag(repo, "b");
    ck_assert_ptr_null(experiment);
    experiment_repository_free(repo);
}

END_TEST

START_TEST (test_experiment_repository_will_return_when_found) {
    List *exp = ROX_LIST(
            experiment_model_create("1", "1", "1", false, ROX_LIST(ROX_COPY("a")), ROX_EMPTY_HASH_SET, "stam"));
    ExperimentRepository *repo = experiment_repository_create();
    experiment_repository_set_experiments(repo, exp);
    ExperimentModel *experiment = experiment_repository_get_experiment_by_flag(repo, "a");
    ck_assert_ptr_nonnull(experiment);
    ck_assert_str_eq(experiment->id, "1");
    experiment_repository_free(repo);
}

END_TEST

START_TEST (test_flag_repository_will_return_null_when_flag_not_found) {
    FlagRepository *repo = flag_repository_create();
    ck_assert_ptr_null(flag_repository_get_flag(repo, "harti"));
    flag_repository_free(repo);
}

END_TEST

START_TEST (test_flag_repository_will_add_flag_and_set_name) {
    FlagRepository *repo = flag_repository_create();
    Variant *flag = variant_create_flag();
    flag_repository_add_flag(repo, flag, "harti");
    Variant *variant = flag_repository_get_flag(repo, "harti");
    ck_assert_ptr_nonnull(variant);
    ck_assert_str_eq(variant->name, "harti");
    flag_repository_free(repo);
}

END_TEST

static Variant *TEST_VARIANT_HANDLER_PROP;

void test_variant_handler(void *target, Variant *variant) {
    TEST_VARIANT_HANDLER_PROP = variant;
}

START_TEST (test_flag_repository_will_raise_flag_added_event) {
    FlagRepository *repo = flag_repository_create();
    Variant *flag = variant_create_flag();
    flag_repository_add_flag_added_callback(repo, NULL, &test_variant_handler);
    flag_repository_add_flag(repo, flag, "harti");
    ck_assert_ptr_nonnull(TEST_VARIANT_HANDLER_PROP);
    ck_assert_str_eq(flag->name, "harti");
    flag_repository_free(repo);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_custom_property_repo_will_return_null_when_prop_not_found),
        ROX_TEST_CASE(test_custom_property_repo_will_add_prop),
        ROX_TEST_CASE(test_custom_property_repo_will_not_override_prop),
        ROX_TEST_CASE(test_custom_property_repo_will_override_prop),
        ROX_TEST_CASE(test_custom_property_repo_will_raise_prop_added_event),
        ROX_TEST_CASE(test_experiment_repository_will_return_null_when_not_found),
        ROX_TEST_CASE(test_experiment_repository_will_return_when_found),
        ROX_TEST_CASE(test_flag_repository_will_return_null_when_flag_not_found),
        ROX_TEST_CASE(test_flag_repository_will_add_flag_and_set_name),
        ROX_TEST_CASE(test_flag_repository_will_raise_flag_added_event)
)