#include <assert.h>
#include <core/consts.h>
#include "impression.h"
#include "util.h"

struct ROX_INTERNAL XImpressionInvoker {
    InternalFlags *flags;
    CustomPropertyRepository *custom_property_repository;
    AnalyticsClient *client;
};

XImpressionInvoker *ROX_INTERNAL x_impression_invoker_create(
        InternalFlags *flags,
        CustomPropertyRepository *custom_property_repository,
        AnalyticsClient *client) {

    assert(flags);
    assert(custom_property_repository);

    XImpressionInvoker *invoker = calloc(1, sizeof(XImpressionInvoker));
    invoker->flags = flags;
    invoker->custom_property_repository = custom_property_repository;
    invoker->client = client;
    return invoker;
}

#define X_IMPRESSION_HANDLER_ROX_PROPERTY_BUFFER_LENGTH 1024

void ROX_INTERNAL x_impression_handler(
        void *target,
        ReportingValue *value,
        Experiment *experiment,
        Context *context) {

    assert(target);

    XImpressionInvoker *invoker = (XImpressionInvoker *) target;
    bool internal_experiment = internal_flags_is_enabled(invoker->flags, "rox.internal.analytics");
    if (value && internal_experiment && experiment && invoker->client) {
        CustomProperty *prop = custom_property_repository_get_custom_property(
                invoker->custom_property_repository, experiment->stickiness_property);
        if (!prop) {
            char buffer[X_IMPRESSION_HANDLER_ROX_PROPERTY_BUFFER_LENGTH];
            char *property_name = str_format_b(
                    buffer, X_IMPRESSION_HANDLER_ROX_PROPERTY_BUFFER_LENGTH, "rox.%s",
                    ROX_PROPERTY_TYPE_DISTINCT_ID.name);
            prop = custom_property_repository_get_custom_property(invoker->custom_property_repository, property_name);
        }
        const char *distinct_id = "(null_distinct_id";
        if (prop) {
            DynamicValue *prop_value = custom_property_get_value(prop, context);
            if (dynamic_value_is_string(prop_value)) {
                char *str = dynamic_value_get_string(prop_value);
                if (str) {
                    distinct_id = str;
                }
            }
        }
        AnalyticsEvent *event = analytics_event_create(
                value->name,
                value->value,
                distinct_id,
                experiment->identifier);
        analytics_client_track(invoker->client, event);
        analytics_event_free(event);
    }
}

#undef X_IMPRESSION_HANDLER_ROX_PROPERTY_BUFFER_LENGTH

void ROX_INTERNAL x_impression_invoker_free(XImpressionInvoker *invoker) {
    assert(invoker);
    free(invoker);
}