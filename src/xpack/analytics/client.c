#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "client.h"
#include "collections.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/consts.h"
#include "core.h"
#include "util.h"

const AnalyticsClientConfig ANALYTICS_CLIENT_INITIAL_CONFIG = {
        NULL,
        NULL,
        1000,
        20,
        false,
        false,
        30,
        30,
        NULL,
        NULL
};

struct AnalyticsClient {
    char *write_key;
    AnalyticsClientConfig *config;
    void *target;
    analytics_client_track_func track;

    RoxList *event_queue;
    pthread_mutex_t queue_mutex;
    int flush_at;
    int max_queue_size;
    int flush_interval_seconds;
    bool flushed_once;
    bool stopped;
    char *analytics_host;
    char *analytics_url;
    DeviceProperties *device_props;

    pthread_t sender_thread;
    pthread_mutex_t sender_mutex;
    pthread_cond_t sender_cond;
    bool flush_requested;
    bool exit_requested;
    bool thread_running;

    PeriodicTask *flush_timer;
};

static cJSON *serialize_event(AnalyticsEvent *event) {
    assert(event);

    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddStringToObject(json, "flag", event->flag);

    if (event->value) {
        cJSON_AddStringToObject(json, "value", event->value);
    }

    cJSON_AddStringToObject(json, "distinctId", event->distinct_id);
    cJSON_AddStringToObject(json, "type", event->type);
    cJSON_AddNumberToObject(json, "time", event->time);

    return json;
}

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

    cJSON_AddStringToObject(payload, "analyticsVersion", "1.0.0");
    cJSON_AddStringToObject(payload, "sdkVersion",
                           device_properties_get_lib_version(device_props));
    cJSON_AddNumberToObject(payload, "time", current_time_millis());
    cJSON_AddStringToObject(payload, "platform", "C");
    cJSON_AddStringToObject(payload, "rolloutKey", client->write_key);

    cJSON *events_array = cJSON_CreateArray();
    if (!events_array) {
        cJSON_Delete(payload);
        return NULL;
    }

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

static char *build_analytics_url(const char *host, const char *write_key) {
    assert(host);
    assert(write_key);

    size_t host_len = strlen(host);
    const char *suffix = "/impression";
    size_t suffix_len = strlen(suffix);

    // Check if host already ends with "/impression"
    bool has_impression = (host_len >= suffix_len &&
                           strcmp(host + host_len - suffix_len, suffix) == 0);

    // Build URL: host + [/impression if missing] + / + writeKey
    const char *middle = has_impression ? "/" : "/impression/";
    size_t url_len = host_len + strlen(middle) + strlen(write_key) + 1;

    char *url = (char *)malloc(url_len);
    if (!url) {
        ROX_ERROR("Failed to allocate memory for analytics URL");
        return NULL;
    }

    snprintf(url, url_len, "%s%s%s", host, middle, write_key);
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
    assert(client->analytics_url);

    ROX_DEBUG("Sending analytics to: %s", client->analytics_url);

    RequestConfig config = DEFAULT_REQUEST_CONFIG_INITIALIZER;
    config.request_timeout = client->config->timeout_seconds > 0
        ? client->config->timeout_seconds : 30;
    Request *request = request_create(&config);

    HttpResponseMessage *response = request_send_post_json(request, client->analytics_url, payload_json);

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

    if (response) {
        response_message_free(response);
    }
    request_free(request);
    cJSON_Delete(payload_json);
    rox_list_free_cb(batch, (void (*)(void *))&analytics_event_free);
}

// Must be called with queue_mutex locked
static void trim_queue(AnalyticsClient *client, int max_size) {
    assert(client);
    assert(client->event_queue);

    while (rox_list_size(client->event_queue) > (size_t)max_size) {
        void *old_event = NULL;
        if (rox_list_get_first(client->event_queue, &old_event) && old_event) {
            rox_list_remove(client->event_queue, old_event);
            analytics_event_free((AnalyticsEvent *)old_event);
            ROX_WARN("Analytics queue full, dropped oldest event");
        } else {
            break;
        }
    }
}

static void enqueue_event(AnalyticsClient *client, AnalyticsEvent *event) {
    assert(client);
    assert(event);
    assert(client->event_queue);

    pthread_mutex_lock(&client->queue_mutex);

    if ((int)rox_list_size(client->event_queue) >= client->max_queue_size) {
        trim_queue(client, client->max_queue_size - 1);

        if ((int)rox_list_size(client->event_queue) >= client->max_queue_size) {
            ROX_ERROR("Failed to trim analytics queue, dropping event");
            analytics_event_free(event);
            pthread_mutex_unlock(&client->queue_mutex);
            return;
        }
    }

    if (!rox_list_add(client->event_queue, event)) {
        ROX_ERROR("Failed to add event to analytics queue");
        analytics_event_free(event);
    } else {
        ROX_DEBUG("Analytics event enqueued, queue size: %zu", rox_list_size(client->event_queue));
    }

    pthread_mutex_unlock(&client->queue_mutex);
}

static bool should_flush(AnalyticsClient *client) {
    assert(client);
    assert(client->event_queue);

    pthread_mutex_lock(&client->queue_mutex);
    bool should = (int)rox_list_size(client->event_queue) >= client->flush_at;
    pthread_mutex_unlock(&client->queue_mutex);

    return should;
}

// Must be called with queue_mutex locked
static RoxList *dequeue_events_for_flush(AnalyticsClient *client, int max_count) {
    assert(client);
    assert(client->event_queue);

    RoxList *batch = rox_list_create();
    if (!batch) {
        return NULL;
    }

    int count = 0;
    size_t queue_size = rox_list_size(client->event_queue);

    while (count < max_count && queue_size > 0) {
        void *event = NULL;
        if (rox_list_get_first(client->event_queue, &event) && event) {
            rox_list_remove(client->event_queue, event);

            if (!rox_list_add(batch, event)) {
                // Event lost, free it
                ROX_ERROR("Failed to add event to flush batch, event lost");
                analytics_event_free((AnalyticsEvent *)event);
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

// Accesses device_props which must remain valid for client lifetime
static void flush_analytics_events(AnalyticsClient *client) {
    assert(client);
    assert(client->event_queue);
    assert(client->device_props);

    pthread_mutex_lock(&client->queue_mutex);

    if (client->stopped) {
        pthread_mutex_unlock(&client->queue_mutex);
        ROX_DEBUG("Analytics: Client stopped, aborting flush");
        return;
    }

    size_t queue_size = rox_list_size(client->event_queue);
    if (queue_size == 0) {
        pthread_mutex_unlock(&client->queue_mutex);
        ROX_DEBUG("Analytics: No events to flush");
        return;
    }

    // Single sender thread guarantees only one flush at a time
    RoxList *batch = dequeue_events_for_flush(client, client->flush_at);
    pthread_mutex_unlock(&client->queue_mutex);

    if (batch && rox_list_size(batch) > 0) {
        ROX_DEBUG("Analytics: Flushing %zu events", rox_list_size(batch));

        cJSON *payload = build_analytics_payload(client, batch, client->device_props);
        if (payload) {
            send_analytics_payload(client, payload, batch);
        } else {
            ROX_ERROR("Failed to build analytics payload");
            rox_list_free_cb(batch, (void (*)(void *))&analytics_event_free);
        }
    } else {
        if (batch) {
            rox_list_free(batch);
        }
    }
}

static void trigger_async_flush(AnalyticsClient *client);

static void periodic_flush_callback(void *target) {
    assert(target);
    AnalyticsClient *client = (AnalyticsClient *)target;

    ROX_DEBUG("Analytics: Periodic flush triggered");
    trigger_async_flush(client);
}

static void *sender_thread_func(void *arg) {
    assert(arg);
    AnalyticsClient *client = (AnalyticsClient *)arg;

    ROX_DEBUG("Analytics sender thread started");

    while (true) {
        pthread_mutex_lock(&client->sender_mutex);

        while (!client->flush_requested && !client->exit_requested) {
            pthread_cond_wait(&client->sender_cond, &client->sender_mutex);
        }

        bool should_flush = client->flush_requested;
        bool should_exit = client->exit_requested;

        if (client->flush_requested) {
            client->flush_requested = false;
        }

        pthread_mutex_unlock(&client->sender_mutex);

        if (should_exit) {
            ROX_DEBUG("Analytics sender thread received exit signal");
            break;
        }

        if (should_flush) {
            flush_analytics_events(client);
        }
    }

    ROX_DEBUG("Analytics sender thread exiting");
    return NULL;
}

static void trigger_async_flush(AnalyticsClient *client) {
    assert(client);

    pthread_mutex_lock(&client->sender_mutex);
    client->flush_requested = true;
    pthread_cond_signal(&client->sender_cond);
    pthread_mutex_unlock(&client->sender_mutex);

    ROX_DEBUG("Analytics: Flush signal sent to sender thread");
}

static void analytics_client_track_impl(void *target, AnalyticsEvent *event) {
    assert(target);
    assert(event);
    AnalyticsClient *client = (AnalyticsClient *)target;

    pthread_mutex_lock(&client->queue_mutex);
    bool is_stopped = client->stopped;
    pthread_mutex_unlock(&client->queue_mutex);

    if (is_stopped) {
        ROX_DEBUG("Analytics: Client stopped, rejecting event");
        return;
    }

    AnalyticsEvent *event_copy = analytics_event_copy(event);
    if (!event_copy) {
        ROX_ERROR("Failed to copy analytics event");
        return;
    }

    enqueue_event(client, event_copy);

    pthread_mutex_lock(&client->queue_mutex);
    bool is_first_flush = !client->flushed_once;
    if (is_first_flush) {
        client->flushed_once = true;
    }
    pthread_mutex_unlock(&client->queue_mutex);

    if (is_first_flush) {
        ROX_DEBUG("Analytics: First event, triggering async flush");
        trigger_async_flush(client);
        return;
    }

    if (should_flush(client)) {
        ROX_DEBUG("Analytics: Queue size reached flush_at (%d), triggering async flush", client->flush_at);
        trigger_async_flush(client);
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

    client->write_key = mem_copy_str(write_key);
    client->config = config;
    client->target = config->target ? config->target : client;
    client->track = config->track_func ? config->track_func : analytics_client_track_impl;

    client->event_queue = rox_list_create();
    if (!client->event_queue) {
        ROX_ERROR("Failed to create analytics event queue");
        free(client->write_key);
        free(client);
        return NULL;
    }

    pthread_mutex_init(&client->queue_mutex, NULL);

    client->flush_at = config->max_batch_size > 0 ? config->max_batch_size : 20;
    client->max_queue_size = config->max_queue_size > 0 ? config->max_queue_size : 1000;
    client->flush_interval_seconds = config->flush_interval_seconds > 0
        ? config->flush_interval_seconds : 30;

    client->flushed_once = false;
    client->stopped = false;

    if (config->host) {
        client->analytics_host = mem_copy_str(config->host);
    } else {
        char buffer[512];
        rox_env_get_analytics_path(buffer, sizeof(buffer));
        client->analytics_host = mem_copy_str(buffer);
    }

    client->analytics_url = build_analytics_url(client->analytics_host, client->write_key);
    if (!client->analytics_url) {
        ROX_ERROR("Failed to build analytics URL");
        free(client->analytics_host);
        free(client->write_key);
        rox_list_free(client->event_queue);
        pthread_mutex_destroy(&client->queue_mutex);
        free(client);
        return NULL;
    }

    client->device_props = properties;

    client->flush_requested = false;
    client->exit_requested = false;
    client->thread_running = false;

    if (pthread_mutex_init(&client->sender_mutex, NULL) != 0) {
        ROX_ERROR("Failed to initialize sender mutex");
        free(client->analytics_url);
        free(client->analytics_host);
        free(client->write_key);
        rox_list_free(client->event_queue);
        pthread_mutex_destroy(&client->queue_mutex);
        free(client);
        return NULL;
    }

    if (pthread_cond_init(&client->sender_cond, NULL) != 0) {
        ROX_ERROR("Failed to initialize sender condition variable");
        pthread_mutex_destroy(&client->sender_mutex);
        free(client->analytics_url);
        free(client->analytics_host);
        free(client->write_key);
        rox_list_free(client->event_queue);
        pthread_mutex_destroy(&client->queue_mutex);
        free(client);
        return NULL;
    }

    if (pthread_create(&client->sender_thread, NULL, sender_thread_func, client) != 0) {
        ROX_ERROR("Failed to create sender thread");
        pthread_cond_destroy(&client->sender_cond);
        pthread_mutex_destroy(&client->sender_mutex);
        free(client->analytics_url);
        free(client->analytics_host);
        free(client->write_key);
        rox_list_free(client->event_queue);
        pthread_mutex_destroy(&client->queue_mutex);
        free(client);
        return NULL;
    }

    client->thread_running = true;

    if (client->flush_interval_seconds > 0) {
        client->flush_timer = periodic_task_create(
            client->flush_interval_seconds,
            client,
            periodic_flush_callback
        );
        if (client->flush_timer) {
            ROX_DEBUG("Analytics periodic timer started (%ds interval)",
                      client->flush_interval_seconds);
        } else {
            ROX_ERROR("Failed to create periodic flush timer - timer disabled");
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

    pthread_mutex_lock(&client->queue_mutex);
    client->stopped = true;
    pthread_mutex_unlock(&client->queue_mutex);

    if (client->flush_timer) {
        ROX_DEBUG("Stopping analytics periodic timer");
        periodic_task_free(client->flush_timer);
        client->flush_timer = NULL;
    }

    if (client->thread_running) {
        ROX_DEBUG("Signaling sender thread to exit");
        pthread_mutex_lock(&client->sender_mutex);
        client->exit_requested = true;
        pthread_cond_signal(&client->sender_cond);
        pthread_mutex_unlock(&client->sender_mutex);

        ROX_DEBUG("Waiting for sender thread to exit");
        pthread_join(client->sender_thread, NULL);
        client->thread_running = false;
        ROX_DEBUG("Sender thread exited cleanly");
    }

    if (client->event_queue) {
        pthread_mutex_lock(&client->queue_mutex);
        size_t event_count = rox_list_size(client->event_queue);
        if (event_count > 0) {
            ROX_WARN("Freeing analytics client with %zu unsent events", event_count);
        }
        rox_list_free_cb(client->event_queue, (void (*)(void *))&analytics_event_free);
        pthread_mutex_unlock(&client->queue_mutex);
    }

    pthread_mutex_destroy(&client->queue_mutex);
    pthread_mutex_destroy(&client->sender_mutex);
    pthread_cond_destroy(&client->sender_cond);

    if (client->write_key) {
        free(client->write_key);
    }
    if (client->analytics_host) {
        free(client->analytics_host);
    }
    if (client->analytics_url) {
        free(client->analytics_url);
    }

    free(client);
}

ROX_INTERNAL int analytics_client_get_queue_size(AnalyticsClient *client) {
    if (!client || !client->event_queue) {
        return 0;
    }

    pthread_mutex_lock(&client->queue_mutex);
    int size = (int)rox_list_size(client->event_queue);
    pthread_mutex_unlock(&client->queue_mutex);

    return size;
}
