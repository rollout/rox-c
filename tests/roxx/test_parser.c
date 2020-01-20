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

void eval_assert_boolean_result(Parser *parser, const char *expr, bool expected_result) {
    assert(parser);
    assert(expr);
    EvaluationResult *result = parser_evaluate_expression(parser, expr, NULL);
    ck_assert(result);
    bool *value = result_get_boolean(result);
    ck_assert(value);
    ck_assert_int_eq(*value, expected_result);
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
    eval_assert_boolean_result(parser, "true", true);
    eval_assert_string_result("red", parser, "\"red\"");
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
    parser_free(parser);
}

END_TEST

START_TEST (test_comparison_expressions_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "lt(500, 100)", false);
    eval_assert_boolean_result(parser, "lt(500, 500)", false);
    eval_assert_boolean_result(parser, "lt(500, 500.54)", true);
    eval_assert_boolean_result(parser, "lte(500, 500)", true);
    eval_assert_boolean_result(parser, "gt(500, 100)", true);
    eval_assert_boolean_result(parser, "gt(500, 500)", false);
    eval_assert_boolean_result(parser, "gt(500.54, 500)", true);
    eval_assert_boolean_result(parser, "gte(500, 500)", true);
    eval_assert_boolean_result(parser, "gte(\"500\", 500)", false);
    parser_free(parser);
}

END_TEST

START_TEST (test_semver_comparison_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "semverLt(\"1.1.0\", \"1.1\")", false);
    eval_assert_boolean_result(parser, "semverLte(\"1.1.0\", \"1.1\")", false);
    eval_assert_boolean_result(parser, "semverGte(\"1.1.0\", \"1.1\")", true);
    eval_assert_boolean_result(parser, "semverEq(\"1.0.0\", \"1\")", false);
//    eval_assert_boolean_result(parser, "semverNe(\"1.0.1\", \"1.0.0.1\")", true); // TODO: revision support
    eval_assert_boolean_result(parser, "semverLt(\"1.1\", \"1.2\")", true);
    eval_assert_boolean_result(parser, "semverLte(\"1.1\", \"1.2\")", true);
    eval_assert_boolean_result(parser, "semverGt(\"1.1.1\", \"1.2\")", false);
    eval_assert_boolean_result(parser, "semverGt(\"1.2.1\", \"1.2\")", true);
    parser_free(parser);
}

END_TEST

START_TEST (test_comparison_with_undefined_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "gte(500, undefined)", false);
    eval_assert_boolean_result(parser, "gt(500, undefined)", false);
    eval_assert_boolean_result(parser, "lte(500, undefined)", false);
    eval_assert_boolean_result(parser, "lt(500, undefined)", false);
    eval_assert_boolean_result(parser, "semverGte(\"1.1\", undefined)", false);
    eval_assert_boolean_result(parser, "semverGt(\"1.1\", undefined)", false);
    eval_assert_boolean_result(parser, "semverLte(\"1.1\", undefined)", false);
    eval_assert_boolean_result(parser, "semverLt(\"1.1\", undefined)", false);
    parser_free(parser);
}

END_TEST

START_TEST (test_unknown_operator_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "NOT_AN_OPERATOR(500, 500)", false);
    eval_assert_boolean_result(parser, "JUSTAWORD(500, 500)", false);
    parser_free(parser);
}

END_TEST

START_TEST (test_undefined_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "isUndefined(undefined)", true);
    eval_assert_boolean_result(parser, "isUndefined(123123)", false);
    eval_assert_boolean_result(parser, "isUndefined(\"undefined\")", false);
    parser_free(parser);
}

END_TEST

START_TEST (test_now_evaluation) {
    Parser *parser = parser_create();
    eval_assert_boolean_result(parser, "gte(now(), now())", true);
    eval_assert_boolean_result(parser, "gte(now(), 2458.123)", true);
    eval_assert_boolean_result(parser, "gte(now(), 1534759307565)", true);
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
    eval_assert_boolean_result(parser, "ifThen(and(true, or(true, true)), true, false)", true);
    eval_assert_boolean_result(parser, "ifThen(and(false, or(true, true)), true, false)", false);
    eval_assert_boolean_result(parser,
                               "ifThen(and(true, or(true, true)), false, ifThen(and(true, or(true, true)), true, true))",
                               false);
    eval_assert_boolean_result(parser,
                               "ifThen(and(false, or(true, true)), false, ifThen(and(true, or(true, true)), true, false))",
                               true);
    eval_assert_boolean_result(parser,
                               "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, false))",
                               false);
    eval_assert_boolean_result(parser,
                               "ifThen(and(false, or(true, true)), false, ifThen(and(true, or(false, false)), false, (and(true,true))))",
                               true);
    eval_assert_boolean_result(parser,
                               "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, (and(true,false))))",
                               false);
    eval_assert_boolean_result_null(parser,
                                    "ifThen(and(false, or(true, true)), true, ifThen(and(true, or(false, false)), true, undefined))");
    parser_free(parser);
}

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_simple_tokenization),
        ROX_TEST_CASE(test_token_type),
        ROX_TEST_CASE(test_simple_expression_evaluation),
        ROX_TEST_CASE(test_eq_expressions_evaluation),
        ROX_TEST_CASE(test_comparison_expressions_evaluation),
        ROX_TEST_CASE(test_semver_comparison_evaluation),
        ROX_TEST_CASE(test_comparison_with_undefined_evaluation),
        ROX_TEST_CASE(test_unknown_operator_evaluation),
        ROX_TEST_CASE(test_undefined_evaluation),
        ROX_TEST_CASE(test_now_evaluation),
        ROX_TEST_CASE(test_if_then_expression_evaluation_string),
        ROX_TEST_CASE(test_if_then_expression_evaluation_int_number),
        ROX_TEST_CASE(test_if_then_expression_evaluation_double_number),
        ROX_TEST_CASE(test_if_then_expression_evaluation_boolean)
)
