#include <check.h>
#include "roxtests.h"
#include "core/context.h"
#include "util.h"
#include "collections.h"

//
// ContextImpTests
//

START_TEST (test_context_will_return_value) {
    RoxContext *context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("a"),
                    rox_dynamic_value_create_string_copy("b")));
    RoxDynamicValue *value = rox_context_get(context, "a");
    ck_assert_ptr_nonnull(value);
    ck_assert_str_eq(rox_dynamic_value_get_string(value), "b");
    rox_dynamic_value_free(value);
    rox_context_free(context);
}

END_TEST

START_TEST (test_context_will_return_null) {
    RoxContext *context = rox_context_create_from_map(ROX_EMPTY_MAP);
    ck_assert_ptr_null(rox_context_get(context, "a"));
    rox_context_free(context);
}

END_TEST

START_TEST (test_context_with_null_map) {
    RoxContext *context = rox_context_create_empty();
    ck_assert_ptr_null(rox_context_get(context, "a"));
    rox_context_free(context);
}

END_TEST

//
// MergedContextTests
//

START_TEST (test_with_null_local_context) {
    RoxContext *global_context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("a"), rox_dynamic_value_create_int(1)));
    RoxContext *merged_context = rox_context_create_merged(global_context, NULL);
    RoxDynamicValue *value = rox_context_get(merged_context, "a");
    ck_assert_int_eq(rox_dynamic_value_get_int(value), 1);
    rox_dynamic_value_free(value);
    ck_assert_ptr_null(rox_context_get(merged_context, "b"));
    rox_context_free(merged_context);
    rox_context_free(global_context);
}

END_TEST

START_TEST (test_with_null_global_context) {
    RoxContext *local_context = rox_context_create_from_map(
            ROX_MAP(mem_copy_str("a"), rox_dynamic_value_create_int(1)));

    RoxContext *merged_context = rox_context_create_merged(NULL, local_context);
    RoxDynamicValue *value = rox_context_get(merged_context, "a");
    ck_assert_int_eq(rox_dynamic_value_get_int(value), 1);
    rox_dynamic_value_free(value);
    ck_assert_ptr_null(rox_context_get(merged_context, "b"));
    rox_context_free(merged_context);
    rox_context_free(local_context);
}

END_TEST

START_TEST (test_with_local_and_global_context) {
    RoxMap *global_map = ROX_MAP(
            mem_copy_str("a"), rox_dynamic_value_create_int(1),
            mem_copy_str("b"), rox_dynamic_value_create_int(2));

    RoxMap *local_map = ROX_MAP(
            mem_copy_str("a"), rox_dynamic_value_create_int(3),
            mem_copy_str("c"), rox_dynamic_value_create_int(4));

    RoxContext *global_context = rox_context_create_from_map(global_map);
    RoxContext *local_context = rox_context_create_from_map(local_map);
    RoxContext *merged_context = rox_context_create_merged(global_context, local_context);

    RoxDynamicValue *v1 = rox_context_get(merged_context, "a");
    RoxDynamicValue *v2 = rox_context_get(merged_context, "b");
    RoxDynamicValue *v3 = rox_context_get(merged_context, "c");
    RoxDynamicValue *v4 = rox_context_get(merged_context, "d");
    ck_assert_int_eq(rox_dynamic_value_get_int(v1), 3);
    ck_assert_int_eq(rox_dynamic_value_get_int(v2), 2);
    ck_assert_int_eq(rox_dynamic_value_get_int(v3), 4);
    ck_assert_ptr_null(v4);

    rox_dynamic_value_free(v3);
    rox_dynamic_value_free(v2);
    rox_dynamic_value_free(v1);

    rox_context_free(merged_context);
    rox_context_free(local_context);
    rox_context_free(global_context);
}

END_TEST

ROX_TEST_SUITE(
// ContextImpTests
        ROX_TEST_CASE(test_context_will_return_value),
        ROX_TEST_CASE(test_context_will_return_null),
        ROX_TEST_CASE(test_context_with_null_map),
// MergedContextTests
        ROX_TEST_CASE(test_with_null_local_context),
        ROX_TEST_CASE(test_with_null_global_context),
        ROX_TEST_CASE(test_with_local_and_global_context)
)
