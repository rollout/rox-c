#include <check.h>

#include "roxtests.h"
#include "roxx/parser.h"

START_TEST (test_simple_tokenization) {

    HashTable *operators;
    hashtable_new(&operators);
    hashtable_add(operators, "eq", "true");
    hashtable_add(operators, "lt", "true");

    List *tokens = tokenized_expression_get_tokens("eq(false, lt(-123, \"123\"))", operators);
    ck_assert_int_eq(list_size(tokens), 5);

    ParserNode *node;
    list_get_at(tokens, 0, (void **) &node);
    ck_assert_int_eq(node->type, NodeTypeRator);
    ck_assert_str_eq("eq", node->str_value);

    list_get_at(tokens, 1, (void **) &node);
    ck_assert_int_eq(node->type, NodeTypeRand);
    ck_assert(!node->is_true);
    ck_assert(node->is_false);

    list_get_at(tokens, 2, (void **) &node);
    ck_assert_int_eq(node->type, NodeTypeRator);
    ck_assert_str_eq("lt", node->str_value);

    list_get_at(tokens, 3, (void **) &node);
    ck_assert_int_eq(node->type, NodeTypeRand);
    ck_assert_ptr_nonnull(node->double_value);
    ck_assert_double_eq(*node->double_value, -123.0);

    list_get_at(tokens, 4, (void **) &node);
    ck_assert_int_eq(node->type, NodeTypeRand);
    ck_assert_ptr_nonnull(node->str_value);
    ck_assert_str_eq(node->str_value, "123");

    list_destroy_cb(tokens, (void (*)(void *)) node_free);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_simple_tokenization));
