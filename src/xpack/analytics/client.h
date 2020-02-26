#pragma once

#include "rollout.h"
#include "model.h"
#include "core/client.h"

//
// Config
//

typedef void (*analytics_client_track_func)(void *target, AnalyticsEvent *event);

typedef struct ROX_INTERNAL AnalyticsClientConfig {
    char *host;
    char *proxy;
    int max_queue_size;
    int max_batch_size;
    bool async;
    bool compress_request;
    int timeout_seconds;
    void *target;
    analytics_client_track_func track_func;
} AnalyticsClientConfig;

extern const AnalyticsClientConfig ANALYTICS_CLIENT_INITIAL_CONFIG;

//
// Client
//

typedef struct ROX_INTERNAL AnalyticsClient AnalyticsClient;

/**
 * @param write_key Not <code>NULL</code>. Value is copied internally.
 * @param config Not <code>NULL</code>.
 * @param properties Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
AnalyticsClient *ROX_INTERNAL analytics_client_create(
        const char *write_key,
        AnalyticsClientConfig *config,
        DeviceProperties *properties);

void ROX_INTERNAL analytics_client_track(AnalyticsClient *client, AnalyticsEvent *event);

void ROX_INTERNAL analytics_client_free(AnalyticsClient *client);
