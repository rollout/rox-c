#include <check.h>
#include "roxtests.h"
#include "core/context.h"
#include "util.h"

//
// ContextImpTests
//

START_TEST (test_context_will_return_value) {
    Context *context = context_create_from_hashtable(
            ROX_HASH_TABLE(mem_copy_str("a"), mem_copy_str("b")));
    ck_assert_str_eq(context_get(context, "a"), "b");
    context_free(context);
}

END_TEST

START_TEST (test_context_will_return_null) {
    Context *context = context_create_from_hashtable(ROX_EMPTY_HASH_TABLE);
    ck_assert_ptr_null(context_get(context, "a"));
    context_free(context);
}

END_TEST

START_TEST (test_context_with_null_map) {
    Context *context = context_create_empty();
    ck_assert_ptr_null(context_get(context, "a"));
    context_free(context);
}

END_TEST

//
// MergedContextTests
//

START_TEST (test_with_null_local_context) {
    Context *global_context = context_create_from_hashtable(
            ROX_HASH_TABLE(mem_copy_str("a"), mem_copy_int(1)));
    Context *merged_context = context_create_merged(global_context, NULL);
    ck_assert_int_eq(*(int *) context_get(merged_context, "a"), 1);
    ck_assert_ptr_null(context_get(merged_context, "b"));
    context_free(merged_context);
    context_free(global_context);
}

END_TEST

START_TEST (test_with_null_global_context) {
    Context *local_context = context_create_from_hashtable(
            ROX_HASH_TABLE(mem_copy_str("a"), mem_copy_int(1)));

    Context *merged_context = context_create_merged(NULL, local_context);
    ck_assert_int_eq(*(int *) context_get(merged_context, "a"), 1);
    ck_assert_ptr_null(context_get(merged_context, "b"));
    context_free(merged_context);
    context_free(local_context);
}

END_TEST

START_TEST (test_with_local_and_global_context) {
    HashTable *globalMap = ROX_HASH_TABLE(
            mem_copy_str("a"), mem_copy_int(1),
            mem_copy_str("b"), mem_copy_int(2));

    HashTable *localMap = ROX_HASH_TABLE(
            mem_copy_str("a"), mem_copy_int(3),
            mem_copy_str("c"), mem_copy_int(4));

    Context *global_context = context_create_from_hashtable(globalMap);
    Context *local_context = context_create_from_hashtable(localMap);
    Context *merged_context = context_create_merged(global_context, local_context);

    ck_assert_int_eq(*(int *) context_get(merged_context, "a"), 3);
    ck_assert_int_eq(*(int *) context_get(merged_context, "b"), 2);
    ck_assert_int_eq(*(int *) context_get(merged_context, "c"), 4);
    ck_assert_ptr_null(context_get(merged_context, "d"));

    context_free(merged_context);
    context_free(local_context);
    context_free(global_context);
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
