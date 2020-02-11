#include <assert.h>
#include "model.h"

AnalyticsEvent *ROX_INTERNAL analytics_event_create(
        const char *flag,
        const char *value,
        const char *distinct_id,
        const char *experiment_id) {

    assert(flag);
    assert(distinct_id);
    assert(experiment_id);

    AnalyticsEvent *event = calloc(1, sizeof(AnalyticsEvent));
    event->experiment_version = mem_copy_str("0");
    event->type = mem_copy_str("IMPRESSION");
    event->time = current_time_millis();
    event->flag = mem_copy_str(flag);
    event->value = value ? mem_copy_str(value) : NULL;
    event->distinct_id = mem_copy_str(distinct_id);
    event->experiment_id = mem_copy_str(experiment_id);
    return event;
}

AnalyticsEvent *ROX_INTERNAL analytics_event_copy(AnalyticsEvent *event) {
    assert(event);
    AnalyticsEvent *copy = calloc(1, sizeof(AnalyticsEvent));
    copy->experiment_version = mem_copy_str(event->experiment_version);
    copy->type = mem_copy_str(event->type);
    copy->time = event->time;
    copy->flag = mem_copy_str(event->flag);
    copy->value = event->value ? mem_copy_str(event->value) : NULL;
    copy->distinct_id = mem_copy_str(event->distinct_id);
    copy->experiment_id = mem_copy_str(event->experiment_id);
    return copy;
}

void ROX_INTERNAL analytics_event_free(AnalyticsEvent *event) {
    assert(event);
    free(event->experiment_id);
    free(event->experiment_version);
    free(event->type);
    free(event->flag);
    free(event->distinct_id);
    if (event->value) {
        free(event->value);
    }
    free(event);
}