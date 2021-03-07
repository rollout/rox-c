#include <rox/freeze.h>
#include "roxtests.h"
#include "fixtures.h"
#include "freeze.h"
#include "core/logging.h"

// FlagFreezeManagerTests

START_TEST (should_not_freeze_when_registering_new_flag_if_freeze_was_not_called) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("test", false);
    ck_assert(!rox_flag_is_frozen(flag));
    ck_assert_int_eq(RoxFreezeNone, rox_flag_get_freeze(flag));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_freeze_when_registering_new_flag_if_freeze_was_called) {
    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    FlagTestFixture *ctx = flag_test_fixture_create_with_options(options);
    RoxStringBase *flag = rox_add_flag("test", false);
    ck_assert(!rox_flag_is_frozen(flag));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(flag));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_freeze_already_registered_flags) {
    RoxStringBase *flag = rox_add_flag("test", false);
    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    FlagTestFixture *ctx = flag_test_fixture_create_with_options(options);
    ck_assert(!rox_flag_is_frozen(flag));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(flag));
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_unfreeze_all_flags) {

    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    FlagTestFixture *ctx = flag_test_fixture_create_with_options(options);

    RoxStringBase *f1 = rox_add_flag("a.b", false);
    RoxStringBase *f2 = rox_add_flag("a", false);
    RoxStringBase *f3 = rox_add_flag("b", false);

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(!rox_flag_is_frozen(f2));
    ck_assert(!rox_flag_is_frozen(f3));

    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f1));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f2));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f3));

    ck_assert(!rox_is_enabled(f1));
    ck_assert(!rox_is_enabled(f2));
    ck_assert(!rox_is_enabled(f3));

    ck_assert(rox_flag_is_frozen(f1));
    ck_assert(rox_flag_is_frozen(f2));
    ck_assert(rox_flag_is_frozen(f3));

    rox_unfreeze();

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(!rox_flag_is_frozen(f2));
    ck_assert(!rox_flag_is_frozen(f3));

    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f1));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f2));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f3));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_unfreeze_flags_in_namespace_only) {

    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    FlagTestFixture *ctx = flag_test_fixture_create_with_options(options);

    RoxStringBase *f1 = rox_add_flag("a.b", false);
    RoxStringBase *f2 = rox_add_flag("c", false);
    RoxStringBase *f3 = rox_add_flag("d", false);

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(!rox_flag_is_frozen(f2));
    ck_assert(!rox_flag_is_frozen(f3));

    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f1));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f2));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f3));

    ck_assert(!rox_is_enabled(f1));
    ck_assert(!rox_is_enabled(f2));
    ck_assert(!rox_is_enabled(f3));

    ck_assert(rox_flag_is_frozen(f1));
    ck_assert(rox_flag_is_frozen(f2));
    ck_assert(rox_flag_is_frozen(f3));

    rox_unfreeze_ns("a");

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(rox_flag_is_frozen(f2));
    ck_assert(rox_flag_is_frozen(f3));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_not_unfreeze_flags_in_different_namespace) {

    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    FlagTestFixture *ctx = flag_test_fixture_create_with_options(options);

    RoxStringBase *f1 = rox_add_flag("a.b", false);
    RoxStringBase *f2 = rox_add_flag("c.d", false);

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(!rox_flag_is_frozen(f2));

    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f1));
    ck_assert_int_eq(RoxFreezeUntilLaunch, rox_flag_get_freeze(f2));

    ck_assert(!rox_is_enabled(f1));
    ck_assert(!rox_is_enabled(f2));

    ck_assert(rox_flag_is_frozen(f1));
    ck_assert(rox_flag_is_frozen(f2));

    rox_unfreeze_ns("a");

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(rox_flag_is_frozen(f2));

    rox_unfreeze_ns("c");

    ck_assert(!rox_flag_is_frozen(f1));
    ck_assert(!rox_flag_is_frozen(f2));

    flag_test_fixture_free(ctx);
}

END_TEST

// DynamicApiTests

START_TEST (test_dynamic_api_is_enabled_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze("test", "val", RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "ifThen(true, \"true\", \"false\")"));

    RoxDynamicApi *api = rox_dynamic_api();
    ck_assert(rox_dynamic_api_is_enabled(api, "test", false)); // freezes the value
    check_impression(ctx, "true");
    rox_check_and_free(rox_dynamic_api_get_string(api, "test", "str"), "true");
    ck_assert_double_eq(0, rox_dynamic_api_get_double(api, "test", 1.2));
    ck_assert_int_eq(0, rox_dynamic_api_get_int(api, "test", 4));

    rox_dynamic_api_free(api);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_dynamic_api_get_string_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze("test", "val", RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "ifThen(true, \"B\", \"A\")"));

    RoxDynamicApi *api = rox_dynamic_api();
    rox_check_and_free(rox_dynamic_api_get_string(api, "test", "A"), "B"); // freezes the value
    check_impression(ctx, "B");
    ck_assert(!rox_dynamic_api_is_enabled(api, "test", false));
    ck_assert_double_eq(0, rox_dynamic_api_get_double(api, "test", 1.2));
    ck_assert_int_eq(0, rox_dynamic_api_get_int(api, "test", 4));
    check_no_impression(ctx);

    rox_dynamic_api_free(api);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_dynamic_api_get_int_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze("test", 8, RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "ifThen(true, \"13\", \"5\")"));

    RoxDynamicApi *api = rox_dynamic_api();
    ck_assert_int_eq(13, rox_dynamic_api_get_int(api, "test", 8)); // freezes the value
    check_impression(ctx, "13");
    ck_assert(!rox_dynamic_api_is_enabled(api, "test", true));
    ck_assert_double_eq(13, rox_dynamic_api_get_double(api, "test", 1.2));
    rox_check_and_free(rox_dynamic_api_get_string(api, "test", "str"), "13");
    check_no_impression(ctx);

    rox_dynamic_api_free(api);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_dynamic_api_get_double_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze("test", 6.1, RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "ifThen(true, \"0.01\", \"19341\")"));

    RoxDynamicApi *api = rox_dynamic_api();
    ck_assert_double_eq(0.01, rox_dynamic_api_get_double(api, "test", 2.2)); // freezes the value
    check_impression(ctx, "0.01");
    ck_assert(!rox_dynamic_api_is_enabled(api, "test", true));
    ck_assert_double_eq(0, rox_dynamic_api_get_int(api, "test", 1));
    rox_check_and_free(rox_dynamic_api_get_string(api, "test", "str"), "0.01");
    check_no_impression(ctx);

    rox_dynamic_api_free(api);
    flag_test_fixture_free(ctx);
}

END_TEST

// RoxDoubleTests

START_TEST (test_freeze_double_flag_with_experiment_wrong_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze("test", 1.1, RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"3.34\""));

    ck_assert_double_eq(3.34, rox_get_double(flag));
    check_impression(ctx, "3.34");
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "3.34");
    check_no_impression(ctx);

    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"true\""));

    ck_assert_double_eq(3.34, rox_get_double(flag));
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "3.34");
    check_no_impression(ctx);

    rox_unfreeze();

    ck_assert_double_eq(1.1, rox_get_double(flag));
    check_impression(ctx, "1.1");
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "1.1");
    check_no_impression(ctx);

    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"2\""));
    rox_unfreeze();

    ck_assert_double_eq(2, rox_get_double(flag));
    check_impression(ctx, "2");
    ck_assert_int_eq(2, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "2");
    check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_double_flag_dependency_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_val = rox_add_string_with_freeze_and_options(
            "colorVal", "red", ROX_LIST_COPY_STR("red", "green", "blue"), RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 1, ROX_DBL_LIST(-2.2, 3.3), RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"-2.2\", \"3.3\")",
            "colorVal", "ifThen(eq(\"-2.2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_get_string(color_val), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);

    rox_check_and_free(rox_get_string(color_val), "blue");
    ck_assert_double_eq(-2.2, rox_get_double(flag));

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_unfreeze_flag(color_val, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(color_val), "red");
    ck_assert_double_eq(1, rox_get_double(flag));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_double_flag_dependency_with_peek_do_not_freeze) {
    // TODO: implement!
}

END_TEST

START_TEST (double_flag_should_use_default_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 1.1, ROX_DBL_LIST(-2.2, 3.3), RoxFreezeUntilLaunch);
    rox_freeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_use_given_freeze_if_none_was_provided_in_double_flag_constructor) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_double("flag", 5.7);
    rox_freeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (double_flag_should_not_freeze_value_by_default) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_options("flag", 2.1, ROX_DBL_LIST(2.1, 3, 4.99));
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_impression(ctx, "2.1");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_double_eq(3, rox_get_double(flag));
    check_impression(ctx, "3");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_double_eq(3, rox_get_double(flag));
    check_impression(ctx, "3");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (double_flag_should_freeze_until_foreground) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 2.1, ROX_DBL_LIST(2.1, 3, 4.99), RoxFreezeUntilForeground);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2.1");
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_impression(ctx, "2.1");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert_double_eq(3, rox_get_double(flag));
    check_impression(ctx, "3");

    flag_test_fixture_set_flag_experiment(ctx, flag, "4.99");
    ck_assert_double_eq(3, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_double_eq(4.99, rox_get_double(flag));
    check_impression(ctx, "4.99");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (double_flag_should_freeze_until_launch) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 2.1, ROX_DBL_LIST(2.1, 3, 4.99), RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2.1");
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_impression(ctx, "2.1");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert_double_eq(2.1, rox_get_double(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_double_eq(3, rox_get_double(flag));
    check_impression(ctx, "3");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (get_double_value_with_freeze_should_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_double_with_freeze_and_options(
            "flag", 2.1, ROX_DBL_LIST(3, 4.99), RoxFreezeUntilLaunch);
    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_double_eq(3, rox_get_double(flag));
    check_impression(ctx, "3");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_double_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (peek_original_double_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

// RoxFlagTests

START_TEST (test_freeze_flag_with_experiment_wrong_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze("test", true, RoxFreezeUntilLaunch);
    flag_test_fixture_set_flag_experiment(ctx, flag, "\"3\"");

    ck_assert(!rox_is_enabled(flag)); // freezes the flag with false
    check_impression(ctx, "false");
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert_double_eq(0, rox_get_double(flag));
    rox_check_and_free(rox_get_string(flag), "false");
    check_no_impression(ctx);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"true\"");
    ck_assert(!rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_use_default_freeze_for_boolean_flag) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilLaunch);
    rox_freeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_use_given_freeze_for_boolean_flag_if_none_was_provided_in_constructor) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag("flag", false);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);
    rox_freeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_not_freeze_boolean_flag_by_default) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag("flag", false);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_set_flag_experiment(ctx, flag, "false");
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_work_with_freeze_none_for_boolean_flag) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_set_flag_experiment(ctx, flag, "false");
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_freeze_boolean_flag_until_foreground) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilForeground);

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_set_flag_experiment(ctx, flag, "false");
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(!rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_freeze_boolean_flag_until_launch) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_set_flag_experiment(ctx, flag, "false");
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert(!rox_is_enabled(flag));
    check_impression(ctx, "false");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (boolean_flag_should_return_special_0_values_on_different_type_freeze_value) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");

    flag_test_fixture_set_flag_experiment(ctx, flag, "false");
    ck_assert(rox_is_enabled(flag));
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert_double_eq(0.0, rox_get_double(flag));
    rox_check_and_free(rox_get_string(flag), "true");
    check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (boolean_flag_is_enabled_should_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_flag_with_freeze("flag", false, RoxFreezeUntilLaunch);
    flag_test_fixture_set_flag_experiment(ctx, flag, "true");
    ck_assert(rox_is_enabled(flag));
    check_impression(ctx, "true");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_boolean_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (get_original_boolean_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (test_complex_boolean_flag_dependency_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_var = rox_add_string_with_freeze_and_options(
            "colorVar", "red", ROX_LIST_COPY_STR("red", "green", "blue"), RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_flag_with_freeze(
            "flag", false, RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"true\", \"false\")",
            "colorVar", "ifThen(eq(\"true\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_get_string(color_var), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);

    rox_check_and_free(rox_get_string(color_var), "blue");
    ck_assert(rox_is_enabled(flag));

    rox_unfreeze_flag(color_var, RoxFreezeUntilLaunch);
    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);

    rox_check_and_free(rox_get_string(color_var), "red");
    ck_assert(!rox_is_enabled(flag));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_boolean_flag_dependency_with_peek_do_not_freeze) {
    // TODO: implement!
}

END_TEST

// RoxIntTests

START_TEST (test_freeze_int_flag_with_experiment_wrong_type) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze("test", 1, RoxFreezeUntilLaunch);
    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"3\""));

    ck_assert_int_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");
    ck_assert_double_eq(3, rox_get_double(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "3");
    check_no_impression(ctx);

    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"true\""));

    ck_assert_int_eq(3, rox_get_int(flag));
    ck_assert_double_eq(3, rox_get_double(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "3");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);

    ck_assert_int_eq(1, rox_get_int(flag));
    check_impression(ctx, "1");
    ck_assert_double_eq(1, rox_get_double(flag));
    ck_assert(!rox_is_enabled(flag));
    rox_check_and_free(rox_get_string(flag), "1");
    check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_int_flag_dependency_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_var = rox_add_string_with_freeze_and_options(
            "colorVar", "red", ROX_LIST_COPY_STR("red", "green", "blue"), RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 1, ROX_INT_LIST(2, 3), RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"2\", \"3\")",
            "colorVar", "ifThen(eq(\"2\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_get_string(color_var), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);

    rox_check_and_free(rox_get_string(color_var), "blue");
    ck_assert_int_eq(2, rox_get_int(flag));

    rox_unfreeze_flag(color_var, RoxFreezeUntilLaunch);
    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);

    rox_check_and_free(rox_get_string(color_var), "red");
    ck_assert_int_eq(1, rox_get_int(flag));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_int_flag_dependency_with_peek_do_not_freeze) {
    // TODO: implement!
}

END_TEST

START_TEST (int_flag_should_use_default_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_int_with_freeze_and_options("flag", 1, ROX_INT_LIST(2), RoxFreezeUntilLaunch);
    rox_freeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (int_flag_should_use_given_freeze_if_none_was_provided_in_constructor) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_int_with_options("flag", 1, ROX_INT_LIST(2));
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);
    rox_freeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (int_flag_should_not_freeze_value_by_default) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_options("flag", 1, ROX_INT_LIST(2, 3));
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2");
    ck_assert_int_eq(2, rox_get_int(flag));

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_int_eq(3, rox_get_int(flag));

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(3, rox_get_int(flag));

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (int_flag_should_work_with_freeze_none) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options("flag", 1, ROX_INT_LIST(2, 3), RoxFreezeNone);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2");
    ck_assert_int_eq(2, rox_get_int(flag));
    check_impression(ctx, "2");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_int_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (int_flag_should_freeze_until_foreground) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeUntilForeground);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2");
    ck_assert_int_eq(2, rox_get_int(flag));
    check_impression(ctx, "2");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_int_eq(2, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(2, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert_int_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");

    flag_test_fixture_set_flag_experiment(ctx, flag, "2");
    ck_assert_int_eq(3, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(2, rox_get_int(flag));
    check_impression(ctx, "2");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (int_flag_should_freeze_until_launch) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_INT_LIST(2, 3, 4),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "2");
    ck_assert_int_eq(2, rox_get_int(flag));
    check_impression(ctx, "2");

    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_int_eq(2, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(2, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    ck_assert_int_eq(2, rox_get_int(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (get_int_value_should_invoke_impression) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_int_with_freeze_and_options(
            "flag", 2, ROX_DBL_LIST(3, 4), RoxFreezeUntilLaunch);
    flag_test_fixture_set_flag_experiment(ctx, flag, "3");
    ck_assert_double_eq(3, rox_get_int(flag));
    check_impression(ctx, "3");
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_int_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (get_original_int_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

// RoxStringTests

START_TEST (test_freeze_string_flag_with_experiment_various_types) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze("test", "red", RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"3\"");
    rox_check_and_free(rox_get_string(flag), "3");
    check_impression(ctx, "3");
    ck_assert_double_eq(3, rox_get_double(flag));
    ck_assert_int_eq(3, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    check_no_impression(ctx);

    flag_test_fixture_set_experiments(ctx, ROX_MAP("test", "\"true\""));
    rox_check_and_free(rox_get_string(flag), "3");
    ck_assert_double_eq(3, rox_get_double(flag));
    ck_assert_int_eq(3, rox_get_int(flag));
    ck_assert(!rox_is_enabled(flag));
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(flag), "true");
    check_impression(ctx, "true");
    ck_assert_double_eq(0, rox_get_double(flag));
    ck_assert_int_eq(0, rox_get_int(flag));
    ck_assert(rox_is_enabled(flag));
    check_no_impression(ctx);

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_use_default_freeze_for_string_flag) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_string_with_freeze("test", "test", RoxFreezeUntilLaunch);
    rox_freeze_flag(flag, RoxFreezeNone);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_use_given_freeze_if_none_was_provided_in_string_flag_constructor) {
    FlagTestFixture *ctx = flag_test_fixture_create();
    RoxStringBase *flag = rox_add_string("test", "test");
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);
    rox_freeze_flag(flag, RoxFreezeUntilLaunch);
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeUntilLaunch);
    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (should_not_freeze_string_value_by_default) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_options("test", "red", ROX_LIST_COPY_STR("red", "blue", "green"));
    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"red\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_impression(ctx, "red");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (string_flag_should_work_with_freeze_none) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "test", "red", ROX_LIST_COPY_STR("red", "blue", "green"), RoxFreezeNone);

    ck_assert_int_eq(rox_flag_get_freeze(flag), RoxFreezeNone);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"red\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_impression(ctx, "red");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (string_flag_should_freeze_until_foreground) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "test", "red", ROX_LIST_COPY_STR("red", "blue", "green"), RoxFreezeUntilForeground);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"red\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_impression(ctx, "red");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    rox_check_and_free(rox_get_string(flag), "red");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"red\"");
    rox_check_and_free(rox_get_string(flag), "blue");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(flag), "red");
    check_impression(ctx, "red");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (string_flag_should_freeze_until_launch) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "test", "red", ROX_LIST_COPY_STR("red", "blue", "green"), RoxFreezeUntilLaunch);

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"red\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_impression(ctx, "red");

    flag_test_fixture_set_flag_experiment(ctx, flag, "\"blue\"");
    rox_check_and_free(rox_get_string(flag), "red");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeNone);
    rox_check_and_free(rox_get_string(flag), "red");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilForeground);
    rox_check_and_free(rox_get_string(flag), "red");
    check_no_impression(ctx);

    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    rox_check_and_free(rox_get_string(flag), "blue");
    check_impression(ctx, "blue");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (peek_current_string_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (get_original_string_value_should_return_frozen_value) {
    // TODO: implement!
}

END_TEST

START_TEST (test_complex_string_flag_dependency_with_freeze) {
    FlagTestFixture *ctx = flag_test_fixture_create();

    RoxStringBase *color_var = rox_add_string_with_freeze_and_options(
            "colorVar", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeUntilLaunch);

    RoxStringBase *flag = rox_add_string_with_freeze_and_options(
            "flag", "good", ROX_LIST_COPY_STR("bad", "ugly"),
            RoxFreezeUntilLaunch);

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            "flag", "ifThen(true, \"bad\", \"ugly\")",
            "colorVar", "ifThen(eq(\"bad\", flagValue(\"flag\")), \"blue\", \"green\")"
    ));

    rox_check_and_free(rox_get_string(color_var), "blue");

    flag_test_fixture_set_experiments(ctx, ROX_EMPTY_MAP);

    rox_check_and_free(rox_get_string(color_var), "blue");
    rox_check_and_free(rox_get_string(flag), "bad");

    rox_unfreeze_flag(color_var, RoxFreezeUntilLaunch);
    rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);

    rox_check_and_free(rox_get_string(color_var), "red");
    rox_check_and_free(rox_get_string(flag), "good");

    flag_test_fixture_free(ctx);
}

END_TEST

START_TEST (test_complex_string_flag_dependency_with_peek_do_not_freeze) {
    // TODO: implement!
}

END_TEST

ROX_TEST_SUITE(
// FlagFreezeManagerTests
        ROX_TEST_CASE(should_not_freeze_when_registering_new_flag_if_freeze_was_not_called),
        ROX_TEST_CASE(should_freeze_when_registering_new_flag_if_freeze_was_called),
        ROX_TEST_CASE(should_freeze_already_registered_flags),
        ROX_TEST_CASE(should_unfreeze_all_flags),
        ROX_TEST_CASE(should_unfreeze_flags_in_namespace_only),
        ROX_TEST_CASE(should_not_unfreeze_flags_in_different_namespace),
// DynamicApiTests
        ROX_TEST_CASE(test_dynamic_api_is_enabled_with_freeze),
        ROX_TEST_CASE(test_dynamic_api_get_string_with_freeze),
        ROX_TEST_CASE(test_dynamic_api_get_int_with_freeze),
        ROX_TEST_CASE(test_dynamic_api_get_double_with_freeze),
// RoxDoubleTests
        ROX_TEST_CASE(test_freeze_double_flag_with_experiment_wrong_type),
        ROX_TEST_CASE(test_complex_double_flag_dependency_with_freeze),
        ROX_TEST_CASE(test_complex_double_flag_dependency_with_peek_do_not_freeze),
        ROX_TEST_CASE(double_flag_should_use_default_freeze),
        ROX_TEST_CASE(should_use_given_freeze_if_none_was_provided_in_double_flag_constructor),
        ROX_TEST_CASE(double_flag_should_not_freeze_value_by_default),
        ROX_TEST_CASE(double_flag_should_freeze_until_foreground),
        ROX_TEST_CASE(double_flag_should_freeze_until_launch),
        ROX_TEST_CASE(get_double_value_with_freeze_should_invoke_impression),
        ROX_TEST_CASE(peek_current_double_value_should_return_frozen_value),
        ROX_TEST_CASE(peek_original_double_value_should_return_frozen_value),
// RoxFlagTests
        ROX_TEST_CASE(test_freeze_flag_with_experiment_wrong_type),
        ROX_TEST_CASE(should_use_default_freeze_for_boolean_flag),
        ROX_TEST_CASE(should_use_given_freeze_for_boolean_flag_if_none_was_provided_in_constructor),
        ROX_TEST_CASE(should_not_freeze_boolean_flag_by_default),
        ROX_TEST_CASE(should_work_with_freeze_none_for_boolean_flag),
        ROX_TEST_CASE(should_freeze_boolean_flag_until_foreground),
        ROX_TEST_CASE(should_freeze_boolean_flag_until_launch),
        ROX_TEST_CASE(boolean_flag_should_return_special_0_values_on_different_type_freeze_value),
        ROX_TEST_CASE(boolean_flag_is_enabled_should_invoke_impression),
        ROX_TEST_CASE(peek_current_boolean_value_should_return_frozen_value),
        ROX_TEST_CASE(get_original_boolean_value_should_return_frozen_value),
        ROX_TEST_CASE(test_complex_boolean_flag_dependency_with_freeze),
        ROX_TEST_CASE(test_complex_boolean_flag_dependency_with_peek_do_not_freeze),
// RoxIntTests
        ROX_TEST_CASE(test_freeze_int_flag_with_experiment_wrong_type),
        ROX_TEST_CASE(test_complex_int_flag_dependency_with_freeze),
        ROX_TEST_CASE(test_complex_int_flag_dependency_with_peek_do_not_freeze),
        ROX_TEST_CASE(int_flag_should_use_default_freeze),
        ROX_TEST_CASE(int_flag_should_use_given_freeze_if_none_was_provided_in_constructor),
        ROX_TEST_CASE(int_flag_should_not_freeze_value_by_default),
        ROX_TEST_CASE(int_flag_should_work_with_freeze_none),
        ROX_TEST_CASE(int_flag_should_freeze_until_foreground),
        ROX_TEST_CASE(int_flag_should_freeze_until_launch),
        ROX_TEST_CASE(get_int_value_should_invoke_impression),
        ROX_TEST_CASE(peek_current_int_value_should_return_frozen_value),
        ROX_TEST_CASE(get_original_int_value_should_return_frozen_value),
// RoxStringTests
        ROX_TEST_CASE(test_freeze_string_flag_with_experiment_various_types),
        ROX_TEST_CASE(should_use_default_freeze_for_string_flag),
        ROX_TEST_CASE(should_use_given_freeze_if_none_was_provided_in_string_flag_constructor),
        ROX_TEST_CASE(should_not_freeze_string_value_by_default),
        ROX_TEST_CASE(string_flag_should_work_with_freeze_none),
        ROX_TEST_CASE(string_flag_should_freeze_until_foreground),
        ROX_TEST_CASE(string_flag_should_freeze_until_launch),
        ROX_TEST_CASE(peek_current_string_value_should_return_frozen_value),
        ROX_TEST_CASE(get_original_string_value_should_return_frozen_value),
        ROX_TEST_CASE(test_complex_string_flag_dependency_with_freeze),
        ROX_TEST_CASE(test_complex_string_flag_dependency_with_peek_do_not_freeze)
)
