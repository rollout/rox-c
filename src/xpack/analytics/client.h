#pragma once

#include "rox/server.h"
#include "model.h"
#include "core/client.h"

//
// Config
//

typedef void (*analytics_client_track_func)(void *target, AnalyticsEvent *event);

typedef struct AnalyticsClientConfig {
    char *host;
    char *proxy;
    int max_queue_size;          // Max events in queue (default: 1000)
    int max_batch_size;           // Flush batch size (default: 20)
    bool async;
    bool compress_request;
    int timeout_seconds;          // HTTP request timeout
    int flush_interval_seconds;   // Auto-flush interval (default: 30)
    void *target;
    analytics_client_track_func track_func;
} AnalyticsClientConfig;

extern const AnalyticsClientConfig ANALYTICS_CLIENT_INITIAL_CONFIG;

//
// Client
//

typedef struct AnalyticsClient AnalyticsClient;

/**
 * Creates an analytics client.
 *
 * @param write_key Not <code>NULL</code>. Value is copied internally.
 * @param config Not <code>NULL</code>.
 * @param properties Not <code>NULL</code>. Must remain valid for the lifetime
 *                   of the AnalyticsClient. The client does not take ownership.
 *                   Caller must ensure properties outlive the client or are freed
 *                   only after analytics_client_free() completes.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL AnalyticsClient *analytics_client_create(
        const char *write_key,
        AnalyticsClientConfig *config,
        DeviceProperties *properties);

ROX_INTERNAL void analytics_client_track(AnalyticsClient *client, AnalyticsEvent *event);

ROX_INTERNAL void analytics_client_free(AnalyticsClient *client);

/**
 * Get current queue size (for testing).
 * @param client Analytics client instance
 * @return Current number of events in queue, or 0 if client is NULL
 */
ROX_INTERNAL int analytics_client_get_queue_size(AnalyticsClient *client);
