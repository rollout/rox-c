#pragma once

#include "roxapi.h"

//
// Event
//

typedef struct ROX_INTERNAL NotificationListenerEvent {
    const char *event_name;
    const char *data;
} NotificationListenerEvent;

/**
 * @param event_name Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
NotificationListenerEvent *ROX_INTERNAL notification_listener_event_create(
        const char *event_name,
        const char *data);

NotificationListenerEvent *ROX_INTERNAL notification_listener_event_copy(NotificationListenerEvent *event);

void ROX_INTERNAL notification_listener_event_free(NotificationListenerEvent *event);

//
// NotificationListener
//

typedef struct ROX_INTERNAL NotificationListener NotificationListener;

typedef void ROX_INTERNAL (*notification_listener_event_handler)(void *target, NotificationListenerEvent *event);

typedef struct ROX_INTERNAL NotificationListenerConfig {
    const char *listen_url;
    const char *app_key;
    bool testing;
    bool current_thread;
} NotificationListenerConfig;

/**
 * @param config Not <code>NULL</code>. String values will be copied internally.
 * @return Not <code>NULL</code>.
 */
NotificationListener *ROX_INTERNAL notification_listener_create(NotificationListenerConfig *config);

/**
 * @param listener Not <code>NULL</code>.
 * @param event_name Not <code>NULL</code>. The value will be copied internally.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_INTERNAL notification_listener_on(
        NotificationListener *listener,
        const char *event_name,
        void *target,
        notification_listener_event_handler handler);

/**
 * FOR UNIT TESTING ONLY.
 *
 * Provide input data and check if the configured handler is invoked.
 *
 * @param listener Not <code>NULL</code>.
 * @param input Not <code>NULL</code>.
 */
void ROX_INTERNAL notification_listener_test(NotificationListener *listener, const char *input);

/**
 * @param listener Not <code>NULL</code>.
 */
void ROX_INTERNAL notification_listener_start(NotificationListener *listener);

/**
 * @param listener Not <code>NULL</code>.
 */
void ROX_INTERNAL notification_listener_stop(NotificationListener *listener);

/**
 * @param listener Not <code>NULL</code>.
 */
void ROX_INTERNAL notification_listener_free(NotificationListener *listener);
