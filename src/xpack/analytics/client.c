#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "client.h"
#include "collections.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/consts.h"
#include "core.h"
#include "util.h"

//
// Client
//

const AnalyticsClientConfig ANALYTICS_CLIENT_INITIAL_CONFIG = {
        NULL,   // host (set from sdk_settings)
        NULL,   // proxy
        1000,   // max_queue_size (from iOS SDK)
        20,     // max_batch_size (flush_at)
        false,  // async
        false,  // compress_request
        30,     // timeout_seconds
        30,     // flush_interval_seconds (from iOS SDK)
        NULL,   // target
        NULL    // track_func
};

struct AnalyticsClient {
    char *write_key;
    AnalyticsClientConfig *config;
    void *target;
    analytics_client_track_func track;

    // Queue management
    RoxList *event_queue;           // Queue of AnalyticsEvent*
    pthread_mutex_t queue_mutex;     // Thread safety
    int flush_at;                    // Max events before flush (default: 20)
    int max_queue_size;              // Max queue size (default: 1000)
    int flush_interval_seconds;      // Time between flushes (default: 30)
    bool flushed_once;               // Track first flush
    bool flush_in_progress;          // Prevent concurrent flushes
    char *analytics_host;            // Analytics endpoint URL
    PeriodicTask *flush_timer;       // Timer for periodic flushing
    DeviceProperties *device_props;  // For payload metadata
};

//
// JSON Serialization
//

/**
 * Serialize single event to JSON object.
 * Returns a cJSON object that must be freed by caller.
 */
static cJSON *serialize_event(AnalyticsEvent *event) {
    assert(event);

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    // Add event fields (matching roxjs format)
    cJSON_AddStringToObject(json, "flag", event->flag);

    if (event->value) {
        cJSON_AddStringToObject(json, "value", event->value);
    }

    cJSON_AddStringToObject(json, "distinctId", event->distinct_id);
    cJSON_AddStringToObject(json, "type", event->type);
    cJSON_AddNumberToObject(json, "time", event->time);

    return json;
}

/**
 * Build complete analytics payload with metadata.
 * Returns a cJSON object that must be freed by caller.
 *
 * Payload format (roxjs):
 * {
 *   "analyticsVersion": "1.0.0",
 *   "sdkVersion": "5.0.0",
 *   "time": 1709000000000,
 *   "platform": "C",
 *   "rolloutKey": "api_key_here",
 *   "events": [...]
 * }
 */
static cJSON *build_analytics_payload(
        AnalyticsClient *client,
        RoxList *events,
        DeviceProperties *device_props) {

    assert(client);
    assert(events);
    assert(device_props);

    cJSON *payload = cJSON_CreateObject();
    if (!payload) {
        return NULL;
    }

    // Add metadata (roxjs format)
    cJSON_AddStringToObject(payload, "analyticsVersion", "1.0.0");
    cJSON_AddStringToObject(payload, "sdkVersion",
                           device_properties_get_lib_version(device_props));
    cJSON_AddNumberToObject(payload, "time", current_time_millis());
    cJSON_AddStringToObject(payload, "platform", "C");
    cJSON_AddStringToObject(payload, "rolloutKey", client->write_key);

    // Add events array
    cJSON *events_array = cJSON_CreateArray();
    if (!events_array) {
        cJSON_Delete(payload);
        return NULL;
    }

    // Serialize each event
    ROX_LIST_FOREACH(item, events, {
        AnalyticsEvent *event = (AnalyticsEvent *)item;
        cJSON *event_json = serialize_event(event);
        if (event_json) {
            cJSON_AddItemToArray(events_array, event_json);
        }
    });

    cJSON_AddItemToObject(payload, "events", events_array);

    return payload;
}

/**
 * Convert cJSON object to JSON string.
 * Returns allocated string that must be freed by caller.
 */
static char *payload_to_json_string(cJSON *payload) {
    assert(payload);

    char *json_str = cJSON_PrintUnformatted(payload);
    if (!json_str) {
        ROX_ERROR("Failed to serialize analytics payload to JSON string");
        return NULL;
    }

    ROX_DEBUG("Analytics JSON payload: %s", json_str);
    return json_str;
}

//
// Network Transmission
//

/**
 * Build analytics URL: {host}/impression/{writeKey}
 * Returns allocated string that must be freed by caller.
 */
static char *build_analytics_url(const char *host, const char *write_key) {
    assert(host);
    assert(write_key);

    // Calculate length: host + "/impression/" + write_key + null terminator
    size_t url_len = strlen(host) + strlen("/impression/") + strlen(write_key) + 1;
    char *url = (char *)malloc(url_len);
    if (!url) {
        ROX_ERROR("Failed to allocate memory for analytics URL");
        return NULL;
    }

    snprintf(url, url_len, "%s/impression/%s", host, write_key);
    return url;
}

/**
 * Send analytics payload to server.
 * Takes ownership of json_payload string (will be freed).
 * Takes ownership of batch list (events will be freed).
 */
static void send_analytics_payload(
        AnalyticsClient *client,
        cJSON *payload_json,
        RoxList *batch) {

    assert(client);
    assert(payload_json);
    assert(batch);

    // Build URL
    char *url = build_analytics_url(client->analytics_host, client->write_key);
    if (!url) {
        ROX_ERROR("Failed to build analytics URL");
        cJSON_Delete(payload_json);
        rox_list_free_cb(batch, (void (*)(void *))&analytics_event_free);
        return;
    }

    ROX_DEBUG("Sending analytics to: %s", url);

    // Create request (using default config)
    RequestConfig config = DEFAULT_REQUEST_CONFIG_INITIALIZER;
    config.request_timeout = client->config->timeout_seconds > 0
        ? client->config->timeout_seconds : 30;
    Request *request = request_create(&config);

    // Send POST request
    HttpResponseMessage *response = request_send_post_json(request, url, payload_json);

    // Handle response
    if (response && response_message_is_successful(response)) {
        ROX_DEBUG("Analytics sent successfully (%d events)", (int)rox_list_size(batch));
        ROX_DEBUG("Response status: %d", response_message_get_status(response));
    } else {
        ROX_WARN("Analytics send failed. Response code: %d",
                 response ? response_message_get_status(response) : 0);
        if (response) {
            char *content = response_get_contents(response);
            if (content) {
                ROX_DEBUG("Response: %s", content);
            }
        }
    }

    // Cleanup
    if (response) {
        response_message_free(response);
    }
    request_free(request);
    free(url);
    cJSON_Delete(payload_json);
    rox_list_free_cb(batch, (void (*)(void *))&analytics_event_free);
}

//
// Queue Operations
//

/**
 * Trim queue to max size - drops oldest events when full.
 * Must be called with queue_mutex locked.
 */
static void trim_queue(AnalyticsClient *client, int max_size) {
    assert(client);
    assert(client->event_queue);

    // Remove oldest events (from front of queue)
    while (rox_list_size(client->event_queue) > (size_t)max_size) {
        void *old_event = NULL;
        if (rox_list_get_first(client->event_queue, &old_event) && old_event) {
            rox_list_remove(client->event_queue, old_event);
            analytics_event_free((AnalyticsEvent *)old_event);
            ROX_WARN("Analytics queue full, dropped oldest event");
        } else {
            break; // Safety: avoid infinite loop if list operations fail
        }
    }
}

/**
 * Add event to queue (thread-safe).
 * Trims queue before adding to prevent overflow.
 */
static void enqueue_event(AnalyticsClient *client, AnalyticsEvent *event) {
    assert(client);
    assert(event);
    assert(client->event_queue);

    pthread_mutex_lock(&client->queue_mutex);

    // Trim queue to max_queue_size - 1 before adding new event
    // This matches iOS SDK behavior: trimQueue(self.queue, 999)
    if ((int)rox_list_size(client->event_queue) >= client->max_queue_size) {
        trim_queue(client, client->max_queue_size - 1);
    }

    if (!rox_list_add(client->event_queue, event)) {
        ROX_ERROR("Failed to add event to analytics queue");
        analytics_event_free(event);
    } else {
        ROX_DEBUG("Analytics event enqueued, queue size: %zu", rox_list_size(client->event_queue));
    }

    pthread_mutex_unlock(&client->queue_mutex);
}

/**
 * Check if flush is needed based on queue size.
 * Thread-safe.
 */
static bool should_flush(AnalyticsClient *client) {
    assert(client);
    assert(client->event_queue);

    pthread_mutex_lock(&client->queue_mutex);
    bool should = (int)rox_list_size(client->event_queue) >= client->flush_at;
    pthread_mutex_unlock(&client->queue_mutex);

    return should;
}

/**
 * Get events from queue for flushing.
 * Must be called with queue_mutex locked.
 * Returns a new list containing up to max_count events.
 * Events are removed from the client's queue.
 */
static RoxList *dequeue_events_for_flush(AnalyticsClient *client, int max_count) {
    assert(client);
    assert(client->event_queue);

    RoxList *batch = rox_list_create();
    if (!batch) {
        return NULL;
    }

    int count = 0;
    size_t queue_size = rox_list_size(client->event_queue);

    // Extract up to max_count events from front of queue
    while (count < max_count && queue_size > 0) {
        void *event = NULL;
        if (rox_list_get_first(client->event_queue, &event) && event) {
            rox_list_remove(client->event_queue, event);

            // Check if add to batch succeeds
            if (!rox_list_add(batch, event)) {
                // Failed to add to batch - event is lost, free it
                ROX_ERROR("Failed to add event to flush batch, event lost");
                analytics_event_free((AnalyticsEvent *)event);
                // Continue trying to dequeue more events
                queue_size--;
                continue;
            }

            count++;
            queue_size--;
        } else {
            break; // No more events
        }
    }

    ROX_DEBUG("Dequeued %d events for flush, %zu remaining in queue", count, rox_list_size(client->event_queue));

    return batch;
}

//
// Flush Logic
//

/**
 * Main flush function - sends batched events to analytics backend.
 * Implements concurrent flush prevention pattern from iOS SDK.
 *
 * Thread safety:
 * - Locks queue_mutex only for queue operations
 * - Network I/O performed outside of mutex lock
 * - Uses flush_in_progress flag to prevent concurrent flushes
 */
static void flush_analytics_events(AnalyticsClient *client) {
    assert(client);
    assert(client->event_queue);
    assert(client->device_props);

    pthread_mutex_lock(&client->queue_mutex);

    // Early return if queue is empty
    size_t queue_size = rox_list_size(client->event_queue);
    if (queue_size == 0) {
        pthread_mutex_unlock(&client->queue_mutex);
        ROX_DEBUG("Analytics: No events to flush");
        return;
    }

    // Prevent concurrent flushes (iOS SDK pattern)
    if (client->flush_in_progress) {
        pthread_mutex_unlock(&client->queue_mutex);
        ROX_DEBUG("Analytics: Flush already in progress, skipping");
        return;
    }

    // Mark flush in progress
    client->flush_in_progress = true;

    // Dequeue batch (up to flush_at events)
    RoxList *batch = dequeue_events_for_flush(client, client->flush_at);

    pthread_mutex_unlock(&client->queue_mutex);

    // Perform network I/O outside of mutex lock
    if (batch && rox_list_size(batch) > 0) {
        ROX_DEBUG("Analytics: Flushing %zu events", rox_list_size(batch));

        // Build payload
        cJSON *payload = build_analytics_payload(client, batch, client->device_props);
        if (payload) {
            // Send to server (takes ownership of payload and batch)
            send_analytics_payload(client, payload, batch);
        } else {
            ROX_ERROR("Failed to build analytics payload");
            // Free batch since send_analytics_payload wasn't called
            rox_list_free_cb(batch, (void (*)(void *))&analytics_event_free);
        }
    } else {
        // Empty batch, just free it
        if (batch) {
            rox_list_free(batch);
        }
    }

    // Mark flush complete
    pthread_mutex_lock(&client->queue_mutex);
    client->flush_in_progress = false;
    pthread_mutex_unlock(&client->queue_mutex);
}

/**
 * Periodic flush callback for timer.
 * Called every flush_interval_seconds.
 */
static void periodic_flush_callback(void *target) {
    assert(target);
    AnalyticsClient *client = (AnalyticsClient *)target;

    ROX_DEBUG("Analytics: Periodic flush triggered");
    flush_analytics_events(client);
}

/**
 * Track event implementation.
 * Adds event to queue and triggers flush based on conditions:
 * - First event: immediate flush
 * - Queue size >= flush_at: flush batch
 * - Time-based: handled by periodic timer
 */
static void analytics_client_track_impl(void *target, AnalyticsEvent *event) {
    assert(target);
    assert(event);
    AnalyticsClient *client = (AnalyticsClient *)target;

    // Copy event (caller might free the original)
    AnalyticsEvent *event_copy = analytics_event_copy(event);
    if (!event_copy) {
        ROX_ERROR("Failed to copy analytics event");
        return;
    }

    // Add to queue (thread-safe)
    enqueue_event(client, event_copy);

    // First event immediate flush (roxjs pattern) - thread-safe check
    pthread_mutex_lock(&client->queue_mutex);
    bool is_first_flush = !client->flushed_once;
    if (is_first_flush) {
        client->flushed_once = true;
    }
    pthread_mutex_unlock(&client->queue_mutex);

    if (is_first_flush) {
        ROX_DEBUG("Analytics: First event, flushing immediately");
        flush_analytics_events(client);
        return;
    }

    // Size-based flush trigger
    if (should_flush(client)) {
        ROX_DEBUG("Analytics: Queue size reached flush_at (%d), flushing", client->flush_at);
        flush_analytics_events(client);
    }
}

ROX_INTERNAL AnalyticsClient *analytics_client_create(
        const char *write_key,
        AnalyticsClientConfig *config,
        DeviceProperties *properties) {

    assert(write_key);
    assert(config);
    assert(properties);

    AnalyticsClient *client = calloc(1, sizeof(AnalyticsClient));
    if (!client) {
        ROX_ERROR("Failed to allocate AnalyticsClient");
        return NULL;
    }

    // Basic fields
    client->write_key = mem_copy_str(write_key);
    client->config = config;
    client->target = config->target ? config->target : client;
    client->track = config->track_func ? config->track_func : analytics_client_track_impl;

    // Queue management - NEW
    client->event_queue = rox_list_create();
    if (!client->event_queue) {
        ROX_ERROR("Failed to create analytics event queue");
        free(client->write_key);
        free(client);
        return NULL;
    }

    pthread_mutex_init(&client->queue_mutex, NULL);

    // Configuration - NEW
    client->flush_at = config->max_batch_size > 0 ? config->max_batch_size : 20;
    client->max_queue_size = config->max_queue_size > 0 ? config->max_queue_size : 1000;
    client->flush_interval_seconds = config->flush_interval_seconds > 0
        ? config->flush_interval_seconds : 30;

    // State - NEW
    client->flushed_once = false;
    client->flush_in_progress = false;

    // Analytics host - NEW
    if (config->host) {
        client->analytics_host = mem_copy_str(config->host);
    } else {
        // Default analytics host from consts
        char buffer[512];
        rox_env_get_analytics_path(buffer, sizeof(buffer));
        client->analytics_host = mem_copy_str(buffer);
    }

    // Device properties reference - NEW
    client->device_props = properties;

    // Start periodic flush timer - NEW
    if (client->flush_interval_seconds > 0) {
        client->flush_timer = periodic_task_create(
            client->flush_interval_seconds,
            client,
            periodic_flush_callback
        );
        if (client->flush_timer) {
            ROX_DEBUG("Analytics periodic timer started (%ds interval)", client->flush_interval_seconds);
        } else {
            ROX_ERROR("Failed to create periodic flush timer - timer disabled");
            // Timer disabled but client still usable
        }
    } else {
        client->flush_timer = NULL;
        ROX_DEBUG("Analytics periodic timer disabled");
    }

    ROX_DEBUG("AnalyticsClient created: flush_at=%d, max_queue=%d, interval=%ds",
              client->flush_at, client->max_queue_size, client->flush_interval_seconds);

    return client;
}

ROX_INTERNAL void analytics_client_track(AnalyticsClient *client, AnalyticsEvent *event) {
    assert(client);
    assert(event);
    client->track(client->target, event);
}

ROX_INTERNAL void analytics_client_free(AnalyticsClient *client) {
    assert(client);

    ROX_DEBUG("Freeing AnalyticsClient");

    // Stop periodic timer first
    if (client->flush_timer) {
        ROX_DEBUG("Stopping analytics periodic timer");
        periodic_task_free(client->flush_timer);
        client->flush_timer = NULL;
    }

    // Reset flush_in_progress flag in case timer was mid-flush
    pthread_mutex_lock(&client->queue_mutex);
    client->flush_in_progress = false;
    pthread_mutex_unlock(&client->queue_mutex);

    // Free queue and all events
    if (client->event_queue) {
        pthread_mutex_lock(&client->queue_mutex);
        size_t event_count = rox_list_size(client->event_queue);
        if (event_count > 0) {
            ROX_WARN("Freeing analytics client with %zu unsent events", event_count);
        }
        rox_list_free_cb(client->event_queue, (void (*)(void *))&analytics_event_free);
        pthread_mutex_unlock(&client->queue_mutex);
    }

    // Destroy mutex
    pthread_mutex_destroy(&client->queue_mutex);

    // Free strings
    if (client->write_key) {
        free(client->write_key);
    }
    if (client->analytics_host) {
        free(client->analytics_host);
    }

    // Free client
    free(client);
}
