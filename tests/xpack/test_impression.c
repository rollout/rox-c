#include <collectc/hashset.h>
#include <assert.h>
#include <check.h>
#include "roxtests.h"
#include "xpack/impression.h"
#include "core/consts.h"

typedef struct ROX_INTERNAL ImpressionValues {
    const char *reporting_value_name;
    const char *reporting_value;
    const char *experiment_id;
    const char *experiment_name;
    const char *experiment_stickiness_property;
    bool experiment_archived;
    HashSet *experiment_labels;
    Context *context;
} ImpressionValues;

static ImpressionValues *_impression_values_create(ReportingValue *value, Experiment *experiment, Context *context) {
    ImpressionValues *values = calloc(1, sizeof(ImpressionValues));
    if (value) {
        values->reporting_value_name = value->name;
        values->reporting_value = value->value;
    }
    if (experiment) {
        values->experiment_id = experiment->identifier;
        values->experiment_name = experiment->name;
        values->experiment_labels = experiment->labels;
        values->experiment_stickiness_property = experiment->stickiness_property;
        values->experiment_archived = experiment->archived;
    }
    values->context = context;
}

static ImpressionValues *_impression_values_check(
        List *list,
        int index,
        ReportingValue *value,
        ExperimentModel *experiment,
        Context *context) {

    ImpressionValues *values;
    ck_assert_int_eq(list_get_at(list, index, (void **) &values), CC_OK);
    ck_assert_str_eq(values->reporting_value_name, value->name);
    ck_assert_str_eq(values->reporting_value, value->value);
    ck_assert_str_eq(values->experiment_id, experiment->id);
    ck_assert_str_eq(values->experiment_name, experiment->name);
    ck_assert_str_eq(values->experiment_stickiness_property, experiment->stickiness_property);
    ck_assert_int_eq(values->experiment_archived, experiment->archived);
    ck_assert_ptr_eq(values->context, context);
    return values;
}

static void _impression_labels_check(ImpressionValues *values, ...) {
    va_list args;
            va_start(args, values);
    HashSetIter iter;
    hashset_iter_init(&iter, values->experiment_labels);
    char *actual_label;
    while (hashset_iter_next(&iter, (void **) &actual_label) != CC_ITER_END) {
        char *expected_label = va_arg(args, char*);
        ck_assert_str_eq(actual_label, expected_label);
    }
            va_end(args);
}

static void _impression_values_free(ImpressionValues *values) {
    assert(values);
    free(values);
}

typedef struct ROX_INTERNAL ImpressionInvocationTestContext {
    ExperimentRepository *experiment_repository;
    Parser *parser;
    InternalFlags *flags;
    CustomPropertyRepository *custom_property_repository;
    RoxOptions *rox_options;
    DeviceProperties *device_properties;
    AnalyticsClient *client;
    XImpressionInvoker *x_invoker;
    ImpressionInvoker *invoker;
    List *impressions;
    List *analytics_events;
} ImpressionInvocationTestContext;

static void _test_impression_handler(void *target, ReportingValue *value, Experiment *experiment, Context *context) {
    assert(target);
    ImpressionInvocationTestContext *ctx = (ImpressionInvocationTestContext *) target;
    list_add(ctx->impressions, _impression_values_create(value, experiment, context));
}

static void _test_analytics_client_track(void *target, AnalyticsEvent *event) {
    assert(target);
    assert(event);
    ImpressionInvocationTestContext *ctx = (ImpressionInvocationTestContext *) target;
    list_add(ctx->analytics_events, analytics_event_copy(event));
}

static ImpressionInvocationTestContext *_impression_test_context_create_cfg(bool roxy) {
    ImpressionInvocationTestContext *ctx = calloc(1, sizeof(ImpressionInvocationTestContext));

    list_new(&ctx->impressions);
    list_new(&ctx->analytics_events);

    ctx->experiment_repository = experiment_repository_create();
    ctx->parser = parser_create();
    ctx->flags = internal_flags_create(ctx->experiment_repository, ctx->parser);
    ctx->custom_property_repository = custom_property_repository_create();
    ctx->rox_options = rox_options_create();

    SdkSettings sdk_settings = {"123", "123"};
    ctx->device_properties = device_properties_create(&sdk_settings, ctx->rox_options);

    if (!roxy) {
        AnalyticsClientConfig config;
        config.target = ctx;
        config.track_func = &_test_analytics_client_track;
        ctx->client = analytics_client_create("test", &config, ctx->device_properties);
    }

    ctx->x_invoker = x_impression_invoker_create(
            ctx->flags, ctx->custom_property_repository, ctx->client);

    ctx->invoker = impression_invoker_create();
    impression_invoker_register(ctx->invoker, ctx->x_invoker, &x_impression_handler);
    impression_invoker_register(ctx->invoker, ctx, &_test_impression_handler);

    return ctx;
}

static ImpressionInvocationTestContext *_impression_test_context_create() {
    return _impression_test_context_create_cfg(false);
}

static ImpressionInvocationTestContext *_impression_test_context_create_roxy() {
    return _impression_test_context_create_cfg(true);
}

static void _impression_test_context_free(ImpressionInvocationTestContext *ctx) {
    assert(ctx);
    impression_invoker_free(ctx->invoker);
    x_impression_invoker_free(ctx->x_invoker);
    analytics_client_free(ctx->client);
    device_properties_free(ctx->device_properties);
    rox_options_free(ctx->rox_options);
    custom_property_repository_free(ctx->custom_property_repository);
    internal_flags_free(ctx->flags);
    parser_free(ctx->parser);
    experiment_repository_free(ctx->experiment_repository);
    list_destroy_cb(ctx->impressions, (void (*)(void *)) &_impression_values_free);
    list_destroy_cb(ctx->analytics_events, (void (*)(void *)) &analytics_event_free);
    free(ctx);
}

//
// ImpressionInvokerTests
//

START_TEST (test_will_set_impression_invoker_empty_invoke_not_throwing_exception) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    impression_invoker_invoke(ctx->invoker, NULL, NULL, NULL);
    _impression_test_context_free(ctx);
}

END_TEST

START_TEST (test_will_test_impression_invoker_invoke_and_parameters) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", true, NULL,
            ROX_SET(mem_copy_str("label1")), "stam");

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);

    ImpressionValues *values = _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);
    _impression_labels_check(values, "label1");

    experiment_model_free(experiment);
    reporting_value_free(reporting_value);
    context_free(context);
    _impression_test_context_free(ctx);
}

END_TEST

START_TEST (test_experiment_constructor) {
    ExperimentModel *original_experiment = experiment_model_create(
            "id", "name", "cond", true, NULL,
            ROX_SET(ROX_COPY("name1")), "stam");

    Experiment *experiment = experiment_create(original_experiment);
    ck_assert_str_eq(original_experiment->name, experiment->name);
    ck_assert_str_eq(original_experiment->id, experiment->identifier);
    ck_assert_int_eq(original_experiment->archived, experiment->archived);

    char *label;
    HashSetIter iter;
    hashset_iter_init(&iter, experiment->labels);
    ck_assert_int_eq(hashset_iter_next(&iter, (void **) &label), CC_OK);
    ck_assert_str_eq(label, "name1");

    experiment_free(experiment);
    experiment_model_free(original_experiment);
}

END_TEST

START_TEST (test_reporting_value_constructor) {
    ReportingValue *reporting_value = reporting_value_create("pi", "ka");
    ck_assert_str_eq("pi", reporting_value->name);
    ck_assert_str_eq("ka", reporting_value->value);
    reporting_value_free(reporting_value);
}

END_TEST

START_TEST (test_will_not_invoke_analytics_when_flag_is_off) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", true, NULL,
            ROX_EMPTY_SET, "stam");

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);
    ck_assert_int_eq(list_size(ctx->analytics_events), 0);

    experiment_model_free(experiment);
    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

START_TEST (test_will_not_invoke_analytics_when_is_roxy) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create_roxy();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "true", false, ROX_LIST_COPY_STR("rox.internal.analytics"),
            ROX_EMPTY_SET, "stam");

    experiment_repository_set_experiments(ctx->experiment_repository, ROX_LIST(experiment));
    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);
    ck_assert_int_eq(list_size(ctx->analytics_events), 0);

    experiment_model_free(experiment);
    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

START_TEST (test_will_invoke_analytics) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    custom_property_repository_add_custom_property(
            ctx->custom_property_repository,
            custom_property_create_using_value(
                    mem_str_format("rox.%s", ROX_PROPERTY_TYPE_DISTINCT_ID.name),
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    dynamic_value_create_string_copy("stam")));

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", true, NULL,
            ROX_EMPTY_SET, "stam");

    experiment_repository_set_experiments(ctx->experiment_repository, ROX_LIST(
            experiment_model_create("id", "name", "true", false, ROX_LIST_COPY_STR("rox.internal.analytics"),
                                    ROX_EMPTY_SET, "test"),
            experiment));

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->analytics_events), 1);

    AnalyticsEvent *event;
    list_get_at(ctx->analytics_events, 0, (void **) &event);
    ck_assert_str_eq(event->distinct_id, "stam");
    ck_assert_str_eq(event->experiment_id, "id");
    ck_assert_str_eq(event->experiment_version, "0");
    ck_assert_str_eq(event->flag, "name");
    ck_assert_str_eq(event->value, "value");
    ck_assert_str_eq(event->type, "IMPRESSION");
    ck_assert_double_le(event->time, current_time_millis());

    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

START_TEST (test_will_invoke_analytics_with_stickiness_prop) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    custom_property_repository_add_custom_property(
            ctx->custom_property_repository,
            custom_property_create_using_value(
                    mem_str_format("rox.%s", ROX_PROPERTY_TYPE_DISTINCT_ID.name),
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    dynamic_value_create_string_copy("stamDist")));

    custom_property_repository_add_custom_property(
            ctx->custom_property_repository,
            custom_property_create_using_value(
                    "stickProp",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    dynamic_value_create_string_copy("stamStick")));

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", false, NULL,
            ROX_EMPTY_SET, "stickProp");

    experiment_repository_set_experiments(ctx->experiment_repository, ROX_LIST(
            experiment_model_create(
                    "id", "name", "true", false, ROX_LIST_COPY_STR("rox.internal.analytics"),
                    ROX_EMPTY_SET, "test"),
            experiment));

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->analytics_events), 1);

    AnalyticsEvent *event;
    list_get_at(ctx->analytics_events, 0, (void **) &event);
    ck_assert_str_eq(event->distinct_id, "stamStick");
    ck_assert_str_eq(event->experiment_id, "id");
    ck_assert_str_eq(event->experiment_version, "0");
    ck_assert_str_eq(event->flag, "name");
    ck_assert_str_eq(event->value, "value");
    ck_assert_str_eq(event->type, "IMPRESSION");
    ck_assert_double_le(event->time, current_time_millis());

    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

START_TEST (test_will_invoke_analytics_with_default_prop_when_no_stickiness_prop) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    custom_property_repository_add_custom_property(
            ctx->custom_property_repository,
            custom_property_create_using_value(
                    mem_str_format("rox.%s", ROX_PROPERTY_TYPE_DISTINCT_ID.name),
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    dynamic_value_create_string_copy("stamDist")));

    custom_property_repository_add_custom_property(
            ctx->custom_property_repository,
            custom_property_create_using_value(
                    "stickProp",
                    &ROX_CUSTOM_PROPERTY_TYPE_STRING,
                    dynamic_value_create_string_copy("stamStick")));

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", false, NULL,
            ROX_EMPTY_SET, "stickPropy");

    experiment_repository_set_experiments(ctx->experiment_repository, ROX_LIST(
            experiment_model_create(
                    "id", "name", "true", false, ROX_LIST_COPY_STR("rox.internal.analytics"),
                    ROX_EMPTY_SET, "test"),
            experiment));

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->analytics_events), 1);

    AnalyticsEvent *event;
    list_get_at(ctx->analytics_events, 0, (void **) &event);
    ck_assert_str_eq(event->distinct_id, "stamDist");
    ck_assert_str_eq(event->experiment_id, "id");
    ck_assert_str_eq(event->experiment_version, "0");
    ck_assert_str_eq(event->flag, "name");
    ck_assert_str_eq(event->value, "value");
    ck_assert_str_eq(event->type, "IMPRESSION");
    ck_assert_double_le(event->time, current_time_millis());

    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

START_TEST (test_will_invoke_analytics_with_bad_distinct_id) {
    ImpressionInvocationTestContext *ctx = _impression_test_context_create();
    Context *context = context_create_from_map(ROX_MAP(ROX_COPY("obj1"), dynamic_value_create_int(1)));
    ReportingValue *reporting_value = reporting_value_create("name", "value");

    ExperimentModel *experiment = experiment_model_create(
            "id", "name", "cond", false, NULL,
            ROX_EMPTY_SET, "stam");

    experiment_repository_set_experiments(ctx->experiment_repository, ROX_LIST(
            experiment_model_create(
                    "id", "name", "true", false, ROX_LIST_COPY_STR("rox.internal.analytics"),
                    ROX_EMPTY_SET, "test"),
            experiment));

    impression_invoker_invoke(ctx->invoker, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->impressions), 1);
    _impression_values_check(ctx->impressions, 0, reporting_value, experiment, context);

    ck_assert_int_eq(list_size(ctx->analytics_events), 1);

    AnalyticsEvent *event;
    list_get_at(ctx->analytics_events, 0, (void **) &event);
    ck_assert_str_eq(event->distinct_id, "(null_distinct_id");
    ck_assert_str_eq(event->experiment_id, "id");
    ck_assert_str_eq(event->experiment_version, "0");
    ck_assert_str_eq(event->flag, "name");
    ck_assert_str_eq(event->value, "value");
    ck_assert_str_eq(event->type, "IMPRESSION");
    ck_assert_double_le(event->time, current_time_millis());

    reporting_value_free(reporting_value);
    context_free(context);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_will_set_impression_invoker_empty_invoke_not_throwing_exception),
        ROX_TEST_CASE(test_will_test_impression_invoker_invoke_and_parameters),
        ROX_TEST_CASE(test_experiment_constructor),
        ROX_TEST_CASE(test_reporting_value_constructor),
        ROX_TEST_CASE(test_will_not_invoke_analytics_when_flag_is_off),
        ROX_TEST_CASE(test_will_not_invoke_analytics_when_is_roxy),
        ROX_TEST_CASE(test_will_invoke_analytics),
        ROX_TEST_CASE(test_will_invoke_analytics_with_stickiness_prop),
        ROX_TEST_CASE(test_will_invoke_analytics_with_default_prop_when_no_stickiness_prop),
        ROX_TEST_CASE(test_will_invoke_analytics_with_bad_distinct_id)
)
