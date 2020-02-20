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
                                                                     rox_dynamic_value_create_string_copy(
                                                                             test_str_value));
    ck_assert_str_eq(custom_property_get_name(prop_string), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_string), &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    rox_check_prop_str(prop_string, test_str_value, NULL);
    custom_property_free(prop_string);

    CustomProperty *prop_double = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                                                                     rox_dynamic_value_create_double(
                                                                             test_double_value));
    ck_assert_str_eq(custom_property_get_name(prop_double), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_double), &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE);
    rox_check_prop_double(prop_double, test_double_value, NULL);
    custom_property_free(prop_double);

    CustomProperty *prop_int = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_INT,
                                                                  rox_dynamic_value_create_int(test_int_value));
    ck_assert_str_eq(custom_property_get_name(prop_int), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_int), &ROX_CUSTOM_PROPERTY_TYPE_INT);
    rox_check_prop_int(prop_int, test_int_value, NULL);
    custom_property_free(prop_int);

    CustomProperty *prop_bool = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
                                                                   rox_dynamic_value_create_boolean(test_bool_value));
    ck_assert_str_eq(custom_property_get_name(prop_bool), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_bool), &ROX_CUSTOM_PROPERTY_TYPE_BOOL);
    rox_check_prop_bool(prop_bool, test_bool_value, NULL);
    custom_property_free(prop_bool);

    CustomProperty *prop_semver = custom_property_create_using_value("prop1", &ROX_CUSTOM_PROPERTY_TYPE_SEMVER,
                                                                     rox_dynamic_value_create_string_copy(
                                                                             test_semver_value));
    ck_assert_str_eq(custom_property_get_name(prop_semver), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_semver), &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    rox_check_prop_str(prop_semver, test_semver_value, NULL);
    custom_property_free(prop_semver);
}

END_TEST

RoxDynamicValue *_test_custom_property_string_value_generator(void *target, RoxContext *context) {
    return rox_dynamic_value_create_string_copy(test_str_value);
}

RoxDynamicValue *_test_custom_property_double_value_generator(void *target, RoxContext *context) {
    return rox_dynamic_value_create_double(test_double_value);
}

RoxDynamicValue *_test_custom_property_int_value_generator(void *target, RoxContext *context) {
    return rox_dynamic_value_create_int(test_int_value);
}

RoxDynamicValue *_test_custom_property_bool_value_generator(void *target, RoxContext *context) {
    return rox_dynamic_value_create_boolean(test_bool_value);
}

RoxDynamicValue *_test_custom_property_semver_value_generator(void *target, RoxContext *context) {
    return rox_dynamic_value_create_string_copy(test_semver_value);
}

START_TEST (test_will_create_property_with_func_value) {

    CustomProperty *prop_string = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                         NULL, &_test_custom_property_string_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_string), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_string), &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    rox_check_prop_str(prop_string, "123", NULL);
    custom_property_free(prop_string);

    CustomProperty *prop_double = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE,
                                                         NULL, &_test_custom_property_double_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_double), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_double), &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE);
    rox_check_prop_double(prop_double, 123.12, NULL);
    custom_property_free(prop_double);

    CustomProperty *prop_int = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_INT,
                                                      NULL, &_test_custom_property_int_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_int), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_int), &ROX_CUSTOM_PROPERTY_TYPE_INT);
    rox_check_prop_int(prop_int, 123, NULL);
    custom_property_free(prop_int);

    CustomProperty *prop_bool = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
                                                       NULL, &_test_custom_property_bool_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_bool), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_bool), &ROX_CUSTOM_PROPERTY_TYPE_BOOL);
    rox_check_prop_bool(prop_bool, true, NULL);
    custom_property_free(prop_bool);

    CustomProperty *prop_semver = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_SEMVER,
                                                         NULL, &_test_custom_property_semver_value_generator);
    ck_assert_str_eq(custom_property_get_name(prop_semver), "prop1");
    ck_assert_ptr_eq(custom_property_get_type(prop_semver), &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    rox_check_prop_str(prop_semver, "1.2.3", NULL);
    custom_property_free(prop_semver);

}

END_TEST

RoxContext *test_context_from_func = NULL;

RoxDynamicValue *_test_capture_context(void *target, RoxContext *context) {
    test_context_from_func = context;
    return rox_dynamic_value_create_string_copy(test_str_value);
}

START_TEST (test_will_pass_context) {
    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("a"),
                    rox_dynamic_value_create_int(test_int_value)));
    CustomProperty *prop_string = custom_property_create("prop1", &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                                                         NULL, &_test_capture_context);
    rox_check_prop_str(prop_string, "123", context);
    ck_assert_ptr_nonnull(test_context_from_func);
    ck_assert_int_eq(rox_dynamic_value_get_int(rox_context_get(test_context_from_func, "a")), test_int_value);
    custom_property_free(prop_string);
    rox_context_free(context);
}

END_TEST

START_TEST (test_device_prop_wil_add_rox_to_the_name) {
    CustomProperty *prop = device_property_create_using_value(
            "prop1", &ROX_CUSTOM_PROPERTY_TYPE_BOOL,
            rox_dynamic_value_create_string_copy(test_str_value));
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