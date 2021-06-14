#include <assert.h>
#include <stdlib.h>
#include "core/consts.h"
#include "impression.h"
#include "util.h"

struct XImpressionInvoker {
    InternalFlags *flags;
    CustomPropertyRepository *custom_property_repository;
    AnalyticsClient *client;
};

ROX_INTERNAL XImpressionInvoker *x_impression_invoker_create(
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

ROX_INTERNAL void x_impression_handler_delegate(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context) {

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
        char *distinct_id = NULL;
        if (prop) {
            RoxDynamicValue *prop_value = custom_property_get_value(prop, context);
            if (rox_dynamic_value_is_string(prop_value)) {
                char *str = rox_dynamic_value_get_string(prop_value);
                if (str) {
                    distinct_id = mem_copy_str(str);
                }
            }
            rox_dynamic_value_free(prop_value);
        }
        AnalyticsEvent *event = analytics_event_create(
                value->name,
                value->value,
                distinct_id ? distinct_id : "(null_distinct_id");
        if (distinct_id) {
            free(distinct_id);
        }
        analytics_client_track(invoker->client, event);
        analytics_event_free(event);
    }
}

#undef X_IMPRESSION_HANDLER_ROX_PROPERTY_BUFFER_LENGTH

ROX_INTERNAL void x_impression_invoker_free(XImpressionInvoker *invoker) {
    assert(invoker);
    free(invoker);
}