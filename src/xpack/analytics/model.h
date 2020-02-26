#pragma once

#include "rollout.h"
#include "util.h"

typedef struct ROX_INTERNAL AnalyticsEvent {
    char *flag;
    char *value;
    char *distinct_id;
    char *experiment_id;
    char *experiment_version;
    char *type;
    double time;
} AnalyticsEvent;

/**
 * @param flag Not <code>NULL</code>.
 * @param value Not <code>NULL</code>?
 * @param distinct_id Not <code>NULL</code>.
 * @param experiment_id Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
AnalyticsEvent *ROX_INTERNAL analytics_event_create(
        const char *flag,
        const char *value,
        const char *distinct_id,
        const char *experiment_id);

/**
 * @param event Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
AnalyticsEvent *ROX_INTERNAL analytics_event_copy(AnalyticsEvent *event);

/**
 * @param event Not <code>NULL</code>.
 */
void ROX_INTERNAL analytics_event_free(AnalyticsEvent *event);
