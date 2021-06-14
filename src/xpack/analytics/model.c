#include <assert.h>
#include <stdlib.h>
#include "model.h"

ROX_INTERNAL AnalyticsEvent *analytics_event_create(const char *flag, const char *value, const char *distinct_id) {

    assert(flag);
    assert(distinct_id);

    AnalyticsEvent *event = calloc(1, sizeof(AnalyticsEvent));
    event->type = mem_copy_str("IMPRESSION");
    event->time = current_time_millis();
    event->flag = mem_copy_str(flag);
    event->value = value ? mem_copy_str(value) : NULL;
    event->distinct_id = mem_copy_str(distinct_id);
    return event;
}

ROX_INTERNAL AnalyticsEvent *analytics_event_copy(AnalyticsEvent *event) {
    assert(event);
    AnalyticsEvent *copy = calloc(1, sizeof(AnalyticsEvent));
    copy->type = mem_copy_str(event->type);
    copy->time = event->time;
    copy->flag = mem_copy_str(event->flag);
    copy->value = event->value ? mem_copy_str(event->value) : NULL;
    copy->distinct_id = mem_copy_str(event->distinct_id);
    return copy;
}

ROX_INTERNAL void analytics_event_free(AnalyticsEvent *event) {
    assert(event);
    free(event->type);
    free(event->flag);
    free(event->distinct_id);
    if (event->value) {
        free(event->value);
    }
    free(event);
}