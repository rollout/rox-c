#define ROX_CLIENT

#include "roxtests.h"
#include "fixtures.h"
#include "rox/overrides.h"
#include "rox/freeze.h"

// FlagOverridesTests

START_TEST (flag_overrides_should_return_null_if_no_override) {
    FlagTestFixture *fixture = flag_test_fixture_create();
    RoxFlagOverrides *overrides = rox_get_overrides();
    ck_assert(!rox_has_override(overrides, "flag1"));
    ck_assert_ptr_null(rox_get_override(overrides, "flag1"));
    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (flag_overrides_should_read_overridden_values) {
    FlagTestFixture *fixture = flag_test_fixture_create_with_storage(ROX_MAP("overrides", "{\"key1\":\"value1\", \"key2\":\"value2\"}"));
    RoxFlagOverrides *overrides = rox_get_overrides();
    ck_assert(rox_has_override(overrides, "key1"));
    ck_assert_str_eq("value1", rox_get_override(overrides, "key1"));
    ck_assert(rox_has_override(overrides, "key2"));
    ck_assert_str_eq("value2", rox_get_override(overrides, "key2"));
    ck_assert(!rox_has_override(overrides, "key3"));
    ck_assert_ptr_null(rox_get_override(overrides, "key3"));
    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (flag_overrides_should_override_flag) {
    FlagTestFixture *fixture = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("test", false);
    ck_assert(!rox_is_enabled(flag));
    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "true");
    ck_assert(rox_has_override(overrides, "test"));
    ck_assert_str_eq("true", rox_get_override(overrides, "test"));
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (clear_overrides_can_be_called_second_time) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *demo_flag = rox_add_flag("demo.demoFlag", false);
    RoxStringBase *demo_int = rox_add_int_with_options("demo.demoInt", 1, ROX_INT_LIST(1, 2, 3));
    RoxStringBase *demo_double = rox_add_double_with_options("demo.demoDouble", 1.1, ROX_DBL_LIST(1.1, 2.2, 3.3));
    RoxStringBase *demo_str = rox_add_string_with_options("demo.demoStr", "red",
                                                          ROX_LIST_COPY_STR("red", "green", "blue"));

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "demo.demoFlag", "true");
    rox_set_override(overrides, "demo.demoInt", "2");
    rox_set_override(overrides, "demo.demoDouble", "3.3");
    rox_set_override(overrides, "demo.demoStr", "blue");

    rox_clear_overrides(overrides);
    rox_clear_overrides(overrides);
    
    flag_test_fixture_free(fixture);
}

END_TEST

// RoxDoubleTests

START_TEST (test_will_check_override_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double("test", 0.8);
    flag_test_fixture_set_flag_experiment(fixture, flag, "4.2");
    ck_assert_double_eq(4.2, rox_get_double(flag));
    flag_test_fixture_check_impression(fixture, "4.2");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "0.64");
    ck_assert_double_eq(0.64, rox_get_double(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_set_override(overrides, "test", "false");
    ck_assert_double_eq(0.0, rox_get_double(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_clear_overrides(overrides);
    ck_assert_double_eq(4.2, rox_get_double(flag));
    flag_test_fixture_check_impression(fixture, "4.2");

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (test_complex_double_flag_dependency_with_override) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"));

    rox_add_double_with_freeze_and_options(
            "flag", 1, ROX_DBL_LIST(-2.2, 3.3), RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"-2.2\", \"3.3\")",
            "colorVal", "ifThen(eq(\"-2.2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "3.3");
    rox_check_and_free(rox_get_string(color_val), "green");

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "\"-2.2\"",
            "colorVal", "ifThen(eq(\"-2.2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));
    rox_check_and_free(rox_get_string(color_val), "green");

    rox_clear_override(overrides, "flag");
    rox_check_and_free(rox_get_string(color_val), "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_double_flag_dependency_with_peek_do_not_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_freeze_and_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 1, ROX_DBL_LIST(-2.2, 3.3),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"-2.2\", \"3.3\")",
            "colorVal", "ifThen(eq(\"-2.2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_peek_current_value(color_val), "blue");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);
    rox_check_and_free(rox_peek_current_value(color_val), "red");
    rox_check_and_free(rox_peek_current_value(flag), "1");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_double_value_should_not_invoke_impression) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_options("test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99));
    flag_test_fixture_set_flag_experiment(fixture, flag, "3.7");
    rox_check_and_free(rox_peek_current_value(flag), "3.7");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_set_flag_experiment(fixture, flag, "4.99");
    rox_check_and_free(rox_peek_current_value(flag), "4.99");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_current_double_value_should_return_overriden_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_options("test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99));
    flag_test_fixture_set_flag_experiment(fixture, flag, "3.7");
    rox_check_and_free(rox_peek_current_value(flag), "3.7");
    flag_test_fixture_check_no_impression(fixture);

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "5");
    rox_check_and_free(rox_peek_current_value(flag), "5");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_current_double_value_should_return_frozen_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(fixture, flag, "2.1");
    rox_check_and_free(rox_peek_current_value(flag), "2.1");
    flag_test_fixture_check_no_impression(fixture);

    rox_get_double(flag); // freezing the value
    flag_test_fixture_check_impression(fixture, "2.1");

    flag_test_fixture_set_flag_experiment(fixture, flag, "3.7");
    rox_check_and_free(rox_peek_current_value(flag), "2.1");
    flag_test_fixture_check_no_impression(fixture);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);

    rox_check_and_free(rox_peek_current_value(flag), "3.7");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_double_value_should_not_invoke_impression) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(fixture, flag, "4.99");
    rox_check_and_free(rox_peek_original_value(flag), "4.99");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_double_value_should_not_return_overriden_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(fixture, flag, "4.99");
    rox_check_and_free(rox_peek_original_value(flag), "4.99");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "0.4");
    rox_check_and_free(rox_peek_original_value(flag), "4.99");

    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_double_value_should_return_frozen_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "test", 2.1, ROX_DBL_LIST(2.1, 3.7, 4.99),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(fixture, flag, "4.99");
    rox_check_and_free(rox_peek_original_value(flag), "4.99");
    flag_test_fixture_check_no_impression(fixture);

    rox_get_double(flag); // freezing the value
    flag_test_fixture_check_impression(fixture, "4.99");

    flag_test_fixture_set_flag_experiment(fixture, flag, "3.7");
    rox_check_and_free(rox_peek_original_value(flag), "4.99");
    flag_test_fixture_check_no_impression(fixture);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_original_value(flag), "3.7");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

// RoxFlagTests

START_TEST (test_will_check_boolean_override_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag("test", false);
    ck_assert(!rox_is_enabled(flag));

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(fixture, "true");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "false");
    ck_assert(!rox_is_enabled(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_set_override(overrides, "test", "5");
    ck_assert(!rox_is_enabled(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_clear_overrides(overrides);
    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(fixture, "true");

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_current_boolean_value_should_not_invoke_impression) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag("test", false);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_current_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_set_flag_experiment(fixture, flag, "false");
    rox_check_and_free(rox_peek_current_value(flag), "false");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_current_boolean_value_should_return_overriden_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag("test", false);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_current_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "false");
    rox_check_and_free(rox_peek_current_value(flag), "false");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_current_boolean_value_should_return_frozen_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze(
            "test", false,
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_current_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    ck_assert(rox_is_enabled(flag));
    flag_test_fixture_check_impression(fixture, "true");

    flag_test_fixture_set_flag_experiment(fixture, flag, "false");
    rox_check_and_free(rox_peek_current_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_current_value(flag), "false");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_boolean_value_should_not_invoke_impression) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze(
            "test", false,
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_original_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_boolean_value_should_not_return_overriden_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze(
            "test", false,
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_original_value(flag), "true");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "false");

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_original_value(flag), "true");

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (peek_original_boolean_value_should_return_frozen_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze(
            "test", false,
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(fixture, flag, "true");
    rox_check_and_free(rox_peek_original_value(flag), "true");

    rox_is_enabled(flag); //freezing the flag
    flag_test_fixture_check_impression(fixture, "true");

    flag_test_fixture_set_flag_experiment(fixture, flag, "false");
    rox_check_and_free(rox_peek_original_value(flag), "true");
    flag_test_fixture_check_no_impression(fixture);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_original_value(flag), "false");
    flag_test_fixture_check_no_impression(fixture);

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (test_complex_boolean_flag_dependency_with_override) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"));

    rox_add_flag("flag", true);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"true\", \"false\")",
            "colorVal", "ifThen(eq(\"true\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "false");
    rox_check_and_free(rox_get_string(color_val), "green");

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "colorVal", "ifThen(eq(\"true\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));
    rox_check_and_free(rox_get_string(color_val), "green");

    rox_clear_override(overrides, "flag");
    rox_check_and_free(rox_get_string(color_val), "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_boolean_flag_dependency_with_peek_do_not_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_freeze_and_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"true\", \"false\")",
            "colorVal", "ifThen(eq(\"true\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_peek_current_value(color_val), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);
    rox_check_and_free(rox_peek_current_value(color_val), "red");
    rox_check_and_free(rox_peek_current_value(flag), "false");

    flag_test_fixture_free(ctx);
}

END_TEST

// RoxIntTests

START_TEST (test_will_check_int_override_value) {
    FlagTestFixture *fixture = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int("test", 1);
    ck_assert_int_eq(1, rox_get_int(flag));

    flag_test_fixture_set_flag_experiment(fixture, flag, "2");
    ck_assert_int_eq(2, rox_get_int(flag));
    flag_test_fixture_check_impression(fixture, "2");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "test", "3");
    ck_assert_int_eq(3, rox_get_int(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_set_override(overrides, "test", "false");
    ck_assert_int_eq(0, rox_get_int(flag));
    flag_test_fixture_check_no_impression(fixture);

    rox_clear_overrides(overrides);
    ck_assert_int_eq(2, rox_get_int(flag));
    flag_test_fixture_check_impression(fixture, "2");

    flag_test_fixture_free(fixture);
}

END_TEST

START_TEST (test_complex_int_flag_dependency_with_override) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"));

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 1, ROX_INT_LIST(2, 3), RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"2\", \"3\")",
            "colorVal", "ifThen(eq(\"2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "3");

    rox_check_and_free(rox_get_string(color_val), "green");

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "\"2\"",
            "colorVal", "ifThen(eq(\"2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));
    rox_check_and_free(rox_get_string(color_val), "green");

    rox_clear_override(overrides, "flag");
    rox_check_and_free(rox_get_string(color_val), "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_int_flag_dependency_with_peek_do_not_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_freeze_and_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 1, ROX_INT_LIST(2, 3),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"2\", \"3\")",
            "colorVal", "ifThen(eq(\"2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_peek_current_value(color_val), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);
    rox_check_and_free(rox_peek_current_value(color_val), "red");
    rox_check_and_free(rox_peek_current_value(flag), "1");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_int_value_should_not_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    rox_check_and_free(rox_peek_current_value(flag), "3");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_set_flag_experiment(ctx, flag, "4");
    rox_check_and_free(rox_peek_current_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_int_value_should_return_overriden_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    rox_check_and_free(rox_peek_current_value(flag), "3");
    flag_test_fixture_check_no_impression(ctx);

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "5");
    rox_check_and_free(rox_peek_current_value(flag), "5");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_int_value_should_return_frozen_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "4");
    rox_check_and_free(rox_peek_current_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    rox_get_int(flag);
    flag_test_fixture_check_impression(ctx, "4");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    rox_check_and_free(rox_peek_current_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_current_value(flag), "3");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_int_value_should_not_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "4");
    rox_check_and_free(rox_peek_original_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_int_value_should_not_return_overriden_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "4");
    rox_check_and_free(rox_peek_original_value(flag), "4");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "3");

    rox_check_and_free(rox_peek_original_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_int_value_should_return_frozen_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "4");
    rox_check_and_free(rox_peek_original_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    rox_get_int(flag);
    flag_test_fixture_check_impression(ctx, "4");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    rox_check_and_free(rox_peek_original_value(flag), "4");
    flag_test_fixture_check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_original_value(flag), "3");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

// RoxStringTests

START_TEST (test_will_check_string_override_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string("flag", "red");
    rox_check_and_free(rox_get_string(flag), "red");
    flag_test_fixture_check_impression(ctx, "red");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_get_string(flag), "green");
    flag_test_fixture_check_impression(ctx, "green");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "blue");
    rox_check_and_free(rox_get_string(flag), "blue");
    flag_test_fixture_check_no_impression(ctx);

    rox_set_override(overrides, "flag", "5");
    rox_check_and_free(rox_get_string(flag), "5");
    flag_test_fixture_check_no_impression(ctx);

    rox_clear_overrides(overrides);
    rox_check_and_free(rox_get_string(flag), "green");
    flag_test_fixture_check_impression(ctx, "green");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_string_value_should_not_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_peek_current_value(flag), "blue");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_current_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_string_value_should_return_overriden_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_current_value(flag), "green");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "blue");
    rox_check_and_free(rox_peek_current_value(flag), "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_string_value_should_return_frozen_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_current_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    rox_check_and_free(rox_get_string(flag), "green");
    flag_test_fixture_check_impression(ctx, "green");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_peek_current_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_current_value(flag), "blue");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_string_value_should_not_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_original_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_string_value_should_not_return_overriden_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_original_value(flag), "green");

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "blue");
    rox_check_and_free(rox_peek_original_value(flag), "green");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_original_string_value_should_return_frozen_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "red", ROX_LIST_COPY_STR("red", "blue", "green"),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"green\"");
    rox_check_and_free(rox_peek_original_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    rox_check_and_free(rox_get_string(flag), "green");
    flag_test_fixture_check_impression(ctx, "green");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_peek_original_value(flag), "green");
    flag_test_fixture_check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_peek_original_value(flag), "blue");
    flag_test_fixture_check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_string_flag_dependency_with_override) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"));

    rox_add_string_with_options("flag", "good", ROX_LIST_COPY_STR("bad", "ugly"));

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"bad\", \"ugly\")",
            "colorVal", "ifThen(eq(\"bad\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "flag", "ugly");
    rox_check_and_free(rox_get_string(color_val), "green");

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "\"bad\"",
            "colorVal", "ifThen(eq(\"bad\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));
    rox_check_and_free(rox_get_string(color_val), "green");

    rox_clear_override(overrides, "flag");
    rox_check_and_free(rox_get_string(color_val), "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_string_flag_dependency_with_peek_do_not_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_freeze_and_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "good", ROX_LIST_COPY_STR("bad", "ugly"),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"bad\", \"ugly\")",
            "colorVal", "ifThen(eq(\"bad\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_peek_current_value(color_val), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);
    rox_check_and_free(rox_peek_current_value(color_val), "red");
    rox_check_and_free(rox_peek_current_value(flag), "good");

    flag_test_fixture_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
// FlagOverridesTests
        ROX_TEST_CASE(flag_overrides_should_return_null_if_no_override),
        ROX_TEST_CASE(flag_overrides_should_read_overridden_values),
        ROX_TEST_CASE(flag_overrides_should_override_flag),
        ROX_TEST_CASE(clear_overrides_can_be_called_second_time),
// RoxDoubleTests
        ROX_TEST_CASE(test_will_check_override_value),
        ROX_TEST_CASE(test_complex_double_flag_dependency_with_override),
        ROX_TEST_CASE(test_complex_double_flag_dependency_with_peek_do_not_freeze),
        ROX_TEST_CASE(peek_current_double_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_current_double_value_should_return_overriden_value),
        ROX_TEST_CASE(peek_current_double_value_should_return_frozen_value),
        ROX_TEST_CASE(peek_original_double_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_original_double_value_should_not_return_overriden_value),
        ROX_TEST_CASE(peek_original_double_value_should_return_frozen_value),
// RoxFlagTests
        ROX_TEST_CASE(test_will_check_boolean_override_value),
        ROX_TEST_CASE(peek_current_boolean_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_current_boolean_value_should_return_overriden_value),
        ROX_TEST_CASE(peek_current_boolean_value_should_return_frozen_value),
        ROX_TEST_CASE(peek_original_boolean_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_original_boolean_value_should_not_return_overriden_value),
        ROX_TEST_CASE(peek_original_boolean_value_should_return_frozen_value),
        ROX_TEST_CASE(test_complex_boolean_flag_dependency_with_override),
        ROX_TEST_CASE(test_complex_boolean_flag_dependency_with_peek_do_not_freeze),
// RoxIntTests
        ROX_TEST_CASE(test_will_check_int_override_value),
        ROX_TEST_CASE(test_complex_int_flag_dependency_with_override),
        ROX_TEST_CASE(test_complex_int_flag_dependency_with_peek_do_not_freeze),
        ROX_TEST_CASE(peek_current_int_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_current_int_value_should_return_overriden_value),
        ROX_TEST_CASE(peek_current_int_value_should_return_frozen_value),
        ROX_TEST_CASE(peek_original_int_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_original_int_value_should_not_return_overriden_value),
        ROX_TEST_CASE(peek_original_int_value_should_return_frozen_value),
// RoxStringTests
        ROX_TEST_CASE(test_will_check_string_override_value),
        ROX_TEST_CASE(peek_current_string_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_current_string_value_should_return_overriden_value),
        ROX_TEST_CASE(peek_current_string_value_should_return_frozen_value),
        ROX_TEST_CASE(peek_original_string_value_should_not_invoke_impression),
        ROX_TEST_CASE(peek_original_string_value_should_not_return_overriden_value),
        ROX_TEST_CASE(peek_original_string_value_should_return_frozen_value),
        ROX_TEST_CASE(test_complex_string_flag_dependency_with_override),
        ROX_TEST_CASE(test_complex_string_flag_dependency_with_peek_do_not_freeze)
)
