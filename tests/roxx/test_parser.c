#include <check.h>
#include <assert.h>
#include <util.h>

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

START_TEST (test_token_type) {

    ck_assert_int_eq(TokenTypeNumber, get_token_type_from_token("123"));
    ck_assert_int_eq(TokenTypeNumber, get_token_type_from_token("-123"));
    ck_assert_int_eq(TokenTypeNumber, get_token_type_from_token("-123.23"));
    ck_assert_int_eq(TokenTypeNumber, get_token_type_from_token("123.23"));

    ck_assert_int_ne(TokenTypeString, get_token_type_from_token("-123"));
    ck_assert_int_eq(TokenTypeString, get_token_type_from_token("\"-123\""));
    ck_assert_int_eq(TokenTypeString, get_token_type_from_token("\"undefined\""));
    ck_assert_int_ne(TokenTypeString, get_token_type_from_token("undefined"));

    ck_assert_int_eq(TokenTypeBool, get_token_type_from_token("false"));
    ck_assert_int_eq(TokenTypeBool, get_token_type_from_token("true"));
    ck_assert_int_ne(TokenTypeBool, get_token_type_from_token("undefined"));

    ck_assert_int_eq(TokenTypeUndefined, get_token_type_from_token("undefined"));
    ck_assert_int_ne(TokenTypeUndefined, get_token_type_from_token("false"));
}

END_TEST

void eval_assert_string_result(Parser *parser, const char *expr, const char *expected_result) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    char *str = result_get_string(result);
    ck_assert(str);
    ck_assert_str_eq(str, expected_result);
    result_free(result);
}

void eval_assert_boolean_result(Parser *parser, const char *expr, bool expected_result) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    bool value = result_get_boolean(result);
    ck_assert(value == expected_result);
    result_free(result);
}

START_TEST (test_simple_expression_evaluation) {
    Parser *parser = parser_create();
    eval_assert_string_result(parser, "true", "true");
    eval_assert_boolean_result(parser, "true", true);
    eval_assert_string_result(parser, "\"red\"", "red");
    eval_assert_boolean_result(parser, "and(true, or(true, true))", true);
    eval_assert_boolean_result(parser, "and(true, or(false, true))", true);
    eval_assert_boolean_result(parser, "not(and(false, or(false, true)))", true);
    parser_free(parser);
}

END_TEST

START_TEST (test_eq_expressions_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "eq(\"la la\", \"la la\")", true);
    eval_assert_boolean_result(parser, "eq(\"la la\", \"la,la\")", false);
    eval_assert_boolean_result(parser, "eq(\"lala\", \"lala\")", true);
    eval_assert_boolean_result(parser, "ne(100.123, 100.321)", true);
    eval_assert_boolean_result(parser, "not(eq(undefined, undefined))", false);
    eval_assert_boolean_result(parser, "not(eq(not(undefined), undefined))", true);
    eval_assert_boolean_result(parser, "not(undefined)", true);
    const char *roxxString = "la \\\"la\\\" la";
    char *expr = mem_str_format("eq(\"%s\", \"la \\\"la\\\" la\")", roxxString);
    eval_assert_boolean_result(parser, expr, true);
    free(expr);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_simple_tokenization),
        ROX_TEST_CASE(test_token_type),
        ROX_TEST_CASE(test_simple_expression_evaluation),
        ROX_TEST_CASE(test_eq_expressions_evaluation))
