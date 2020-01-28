#include <check.h>
#include "roxtests.h"
#include "core/properties.h"
#include "core/context.h"
#include "util.h"

//
// CustomPropertyTests
//

static const char *test_str_value = "123";
static const double test_double_value = 123.12;
static const int test_int_value = 123;
static const int test_bool_value = true;
static const char *test_semver_value = "1.2.3";

START_TEST (test_will_create_property_with_const_value) {

    CustomProperty *prop_string = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                                     (void *) test_str_value);
    ck_assert_str_eq(custom_property_get_name(prop_string), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_string), &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    ck_assert_str_eq(custom_property_get_value(prop_string, NULL), test_str_value);
    custom_property_free(prop_string);

    CustomProperty *prop_double = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                                                                     (void *) &test_double_value);
    ck_assert_str_eq(custom_property_get_name(prop_double), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_double), &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE);
    ck_assert_double_eq(*(double *) custom_property_get_value(prop_double, NULL), test_double_value);
    custom_property_free(prop_double);

    CustomProperty *prop_int = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_INT,
                                                                  (void *) &test_int_value);
    ck_assert_str_eq(custom_property_get_name(prop_int), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_int), &ROX_CUSTOM_PROPERTY_TYPE_INT);
    ck_assert_int_eq(*(int *) custom_property_get_value(prop_int, NULL), test_int_value);
    custom_property_free(prop_int);

    CustomProperty *prop_bool = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL,
                                                                   (void *) &test_bool_value);
    ck_assert_str_eq(custom_property_get_name(prop_bool), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_bool), &ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL);
    ck_assert_int_eq(*(bool *) custom_property_get_value(prop_bool, NULL), test_bool_value);
    custom_property_free(prop_bool);

    CustomProperty *prop_semver = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_SEMVER,
                                                                     (void *) test_semver_value);
    ck_assert_str_eq(custom_property_get_name(prop_semver), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_semver), &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    ck_assert_str_eq(custom_property_get_value(prop_semver, NULL), test_semver_value);
    custom_property_free(prop_semver);
}

END_TEST

void *_test_custom_property_string_value_generator(Context *context) {
    return (void *) test_str_value;
}

void *_test_custom_property_double_value_generator(Context *context) {
    return (void *) &test_double_value;
}

void *_test_custom_property_int_value_generator(Context *context) {
    return (void *) &test_int_value;
}

void *_test_custom_property_bool_value_generator(Context *context) {
    return (void *) &test_bool_value;
}

void *_test_custom_property_semver_value_generator(Context *context) {
    return (void *) test_semver_value;
}

START_TEST (test_will_create_property_with_func_value) {

    CustomProperty *prop_string = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                         &_test_custom_property_string_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_string), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_string), &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    ck_assert_str_eq((char *) custom_property_get_value(prop_string, NULL), "123");
    custom_property_free(prop_string);

    CustomProperty *prop_double = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                                                         &_test_custom_property_double_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_double), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_double), &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE);
    ck_assert_double_eq(*(double *) custom_property_get_value(prop_double, NULL), 123.12);
    custom_property_free(prop_double);

    CustomProperty *prop_int = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_INT,
                                                      &_test_custom_property_int_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_int), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_int), &ROX_CUSTOM_PROPERTY_TYPE_INT);
    ck_assert_int_eq(*(int *) custom_property_get_value(prop_int, NULL), 123);
    custom_property_free(prop_int);

    CustomProperty *prop_bool = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL,
                                                       &_test_custom_property_bool_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_bool), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_bool), &ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL);
    ck_assert_int_eq(*(bool *) custom_property_get_value(prop_bool, NULL), true);
    custom_property_free(prop_bool);

    CustomProperty *prop_semver = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_SEMVER,
                                                         &_test_custom_property_semver_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_semver), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_semver), &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    ck_assert_str_eq((char *) custom_property_get_value(prop_semver, NULL), "1.2.3");
    custom_property_free(prop_semver);

}

END_TEST

Context *test_context_from_func = NULL;

void *_test_capture_context(Context *context) {
    test_context_from_func = context;
    return (void *) test_str_value;
}

START_TEST (test_will_pass_context) {
    Context *context = context_create_from_hashtable(
            ROX_HASH_TABLE(mem_copy_str("a"), mem_copy_int(test_int_value)));
    CustomProperty *prop_string = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                         &_test_capture_context);
    ck_assert_str_eq(custom_property_get_value(prop_string, context), "123");
    ck_assert_ptr_nonnull(test_context_from_func);
    ck_assert_int_eq(*(int *) context_get(test_context_from_func, "a"), test_int_value);
    custom_property_free(prop_string);
    context_free(context);
}

END_TEST

START_TEST (test_device_prop_wil_add_rox_to_the_name) {
    CustomProperty *prop = device_property_create_using_value(
            "prop1", &ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL, (void *) test_str_value);
    ck_assert_str_eq(custom_property_get_name(prop), "rox.prop1");
    custom_property_free(prop);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_will_create_property_with_const_value),
        ROX_TEST_CASE(test_will_create_property_with_func_value),
        ROX_TEST_CASE(test_will_pass_context),
        ROX_TEST_CASE(test_device_prop_wil_add_rox_to_the_name)
)