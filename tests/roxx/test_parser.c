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

void eval_assert_string_result(const char *expected_result, Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    char *str = result_get_string(result);
    if (expected_result) {
        ck_assert_str_eq(str, expected_result);
    } else {
        ck_assert_ptr_null(str);
    }
    result_free(result);
}

void eval_assert_boolean_result(bool expected_result, Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    bool *value = result_get_boolean(result);
    ck_assert(value);
    ck_assert(*value == expected_result);
    result_free(result);
}

void eval_assert_boolean_result_null(Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    bool *value = result_get_boolean(result);
    ck_assert_ptr_null(value);
    result_free(result);
}

void eval_assert_int_result(int expected_result, Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    int *value = result_get_int(result);
    assert(value);
    ck_assert_int_eq(*value, expected_result);
    result_free(result);
}

void eval_assert_int_result_null(Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    int *value = result_get_int(result);
    ck_assert_ptr_null(value);
    result_free(result);
}

void eval_assert_double_result(double expected_result, Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    double *value = result_get_double(result);
    assert(value);
    ck_assert_double_eq(*value, expected_result);
    result_free(result);
}

void eval_assert_double_result_null(Parser *parser, const char *expr) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    double *value = result_get_double(result);
    ck_assert_ptr_null(value);
    result_free(result);
}

START_TEST (test_simple_expression_evaluation) {
    Parser *parser = parser_create();
    eval_assert_string_result("true", parser, "true");
    eval_assert_boolean_result(true, parser, "true");
    eval_assert_string_result("red", parser, "\"red\"");
    eval_assert_boolean_result(true, parser, "and(true, or(true, true))");
    eval_assert_boolean_result(true, parser, "and(true, or(false, true))");
    eval_assert_boolean_result(true, parser, "not(and(false, or(false, true)))");
    parser_free(parser);
}

END_TEST

START_TEST (test_eq_expressions_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(true, parser, "eq(\"la la\", \"la la\")");
    eval_assert_boolean_result(false, parser, "eq(\"la la\", \"la,la\")");
    eval_assert_boolean_result(true, parser, "eq(\"lala\", \"lala\")");
    eval_assert_boolean_result(true, parser, "ne(100.123, 100.321)");
    eval_assert_boolean_result(false, parser, "not(eq(undefined, undefined))");
    eval_assert_boolean_result(true, parser, "not(eq(not(undefined), undefined))");
    eval_assert_boolean_result(true, parser, "not(undefined)");
    const char *roxxString = "la \\\"la\\\" la";
    char *expr = mem_str_format("eq(\"%s\", \"la \\\"la\\\" la\")", roxxString);
    eval_assert_boolean_result(true, parser, expr);
    free(expr);
    parser_free(parser);
}

END_TEST

START_TEST (test_unknown_operator_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(false, parser, "NOT_AN_OPERATOR(500, 500)");
    eval_assert_boolean_result(false, parser, "JUSTAWORD(500, 500)");
    parser_free(parser);
}

END_TEST

START_TEST (test_undefined_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(true, parser, "isUndefined(undefined)");
    eval_assert_boolean_result(false, parser, "isUndefined(123123)");
    eval_assert_boolean_result(false, parser, "isUndefined(\"undefined\")");
    parser_free(parser);
}

END_TEST

START_TEST (test_now_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(true, parser, "gte(now(), now())");
    eval_assert_boolean_result(true, parser, "gte(now(), 2458.123)");
    eval_assert_boolean_result(true, parser, "gte(now(), 1534759307565)");
    parser_free(parser);
}

END_TEST

START_TEST (test_if_then_expression_evaluation_string) {
    Parser *parser = parser_create();
    eval_assert_string_result("AB", parser, "ifThen(and(true, or(true, true)), \"AB\", \"CD\")");
    eval_assert_string_result("CD", parser, "ifThen(and(false, or(true, true)), \"AB\", \"CD\")");
    eval_assert_string_result(
            "AB", parser,
            "ifThen(and(true, or(true, true)), \"AB\", ifThen(and(true, or(true, true)), \"EF\", \"CD\"))");
    eval_assert_string_result(
            "EF", parser,
            "ifThen(and(false, or(true, true)), \"AB\", ifThen(and(true, or(true, true)), \"EF\", \"CD\"))");
    eval_assert_string_result(
            "CD", parser,
            "ifThen(and(false, or(true, true)), \"AB\", ifThen(and(true, or(false, false)), \"EF\", \"CD\"))");
    eval_assert_string_result(
            NULL, parser,
            "ifThen(and(false, or(true, true)), \"AB\", ifThen(and(true, or(false, false)), \"EF\", undefined))");
    parser_free(parser);
}

END_TEST

START_TEST (test_if_then_expression_evaluation_int_number) {
    Parser *parser = parser_create();
    eval_assert_int_result(1, parser, "ifThen(and(true, or(true, true)), 1, 2)");
    eval_assert_int_result(2, parser, "ifThen(and(false, or(true, true)), 1, 2)");
    eval_assert_int_result(1, parser,
                           "ifThen(and(true, or(true, true)), 1, ifThen(and(true, or(true, true)), 3, 2))");
    eval_assert_int_result(3, parser,
                           "ifThen(and(false, or(true, true)), 1, ifThen(and(true, or(true, true)), 3, 2))");
    eval_assert_int_result(2, parser,
                           "ifThen(and(false, or(true, true)), 1, ifThen(and(true, or(false, false)), 3, 2))");
    eval_assert_int_result_null(parser,
                                "ifThen(and(false, or(true, true)), 1, ifThen(and(true, or(false, false)), 3, undefined))");
    parser_free(parser);
}

END_TEST

START_TEST (test_if_then_expression_evaluation_double_number) {
    Parser *parser = parser_create();
    eval_assert_double_result(1.1, parser, ("ifThen(and(true, or(true, true)), 1.1, 2.2)"));
    eval_assert_double_result(2.2, parser, ("ifThen(and(false, or(true, true)), 1.1, 2.2)"));
    eval_assert_double_result(1.1, parser, (
            "ifThen(and(true, or(true, true)), 1.1, ifThen(and(true, or(true, true)), 3.3, 2.2))"));
    eval_assert_double_result(3.3, parser, (
            "ifThen(and(false, or(true, true)), 1.1, ifThen(and(true, or(true, true)), 3.3, 2.2))"));
    eval_assert_double_result(2.2, parser, (
            "ifThen(and(false, or(true, true)), 1.1, ifThen(and(true, or(false, false)), 3.3, 2.2))"));
    eval_assert_double_result_null(parser, (
            "ifThen(and(false, or(true, true)), 1.1, ifThen(and(true, or(false, false)), 3.3, undefined))"));
    parser_free(parser);
}

END_TEST

START_TEST (test_if_then_expression_evaluation_boolean) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(true, parser, "ifThen(and(true, or(true, true)), true, false)");
    eval_assert_boolean_result(false, parser, "ifThen(and(false, or(true, true)), true, false)");
    eval_assert_boolean_result(false, parser,
                               "ifThen(and(true, or(true, true)), false, ifThen(and(true, or(true, true)), true, true))");
    eval_assert_boolean_result(true, parser,
                               "ifThen(and(false, or(true, true)), false, ifThen(and(true, or(true, true)), true, false))");
    eval_assert_boolean_result(false, parser,
                               "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, false))");
    eval_assert_boolean_result(true, parser,
                               "ifThen(and(false, or(true, true)), false, ifThen(and(true, or(false, false)), false, (and(true,true))))");
    eval_assert_boolean_result(false, parser,
                               "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, (and(true,false))))");
    eval_assert_boolean_result_null(parser,
                                    "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, undefined))");
    parser_free(parser);
}

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_simple_tokenization),
        ROX_TEST_CASE(test_token_type),
        ROX_TEST_CASE(test_simple_expression_evaluation),
        ROX_TEST_CASE(test_eq_expressions_evaluation),
        ROX_TEST_CASE(test_unknown_operator_evaluation),
        ROX_TEST_CASE(test_undefined_evaluation),
//ROX_TEST_CASE(test_now_evaluation), // FIXME: implement gte operator
        ROX_TEST_CASE(test_if_then_expression_evaluation_string),
        ROX_TEST_CASE(test_if_then_expression_evaluation_int_number),
        ROX_TEST_CASE(test_if_then_expression_evaluation_double_number),
        ROX_TEST_CASE(test_if_then_expression_evaluation_boolean)
)
