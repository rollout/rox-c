#include <assert.h>
#include "configuration.h"
#include "notifications.h"
#include "core/consts.h"

struct ROX_INTERNAL XConfigurationFetchedInvoker {
    InternalFlags *flags;
    SdkSettings *sdk_settings;
    void *fetch_target;
    x_configuration_fetch_func fetch_func;
    NotificationListener *push_updates_listener;
};

XConfigurationFetchedInvoker *ROX_INTERNAL x_configuration_fetched_invoker_create(
        InternalFlags *flags,
        SdkSettings *sdk_settings,
        void *fetch_target,
        x_configuration_fetch_func fetch_func) {

    assert(flags);
    assert(sdk_settings);
    assert(fetch_func);

    XConfigurationFetchedInvoker *invoker = calloc(1, sizeof(XConfigurationFetchedInvoker));
    invoker->flags = flags;
    invoker->sdk_settings = sdk_settings;
    invoker->fetch_target = fetch_target;
    invoker->fetch_func = fetch_func;
    return invoker;
}

static void _config_notification_listener_event_handler(void *target, NotificationListenerEvent *event) {
    assert(target);
    assert(event);
    XConfigurationFetchedInvoker *invoker = (XConfigurationFetchedInvoker *) target;
    invoker->fetch_func(invoker->fetch_target, true);
}

#define X_CONF_FETCH_NOTIFICATIONS_PATH_BUFFER_SIZE 1024

static void _start_or_stop_push_updates_listener(XConfigurationFetchedInvoker *invoker) {
    assert(invoker);

    if (internal_flags_is_enabled(invoker->flags, "rox.internal.pushUpdates")) {
        if (!invoker->push_updates_listener) {
            char notifications_path[X_CONF_FETCH_NOTIFICATIONS_PATH_BUFFER_SIZE];
            rox_env_get_notifications_path(notifications_path, X_CONF_FETCH_NOTIFICATIONS_PATH_BUFFER_SIZE);
            NotificationListenerConfig config;
            config.listen_url = notifications_path;
            config.app_key = invoker->sdk_settings->api_key;
            invoker->push_updates_listener = notification_listener_create(&config);
            notification_listener_on(
                    invoker->push_updates_listener, "changed", invoker,
                    &_config_notification_listener_event_handler);
            notification_listener_start(invoker->push_updates_listener);
        }
    } else {
        if (invoker->push_updates_listener) {
            notification_listener_stop(invoker->push_updates_listener);
            notification_listener_free(invoker->push_updates_listener);
            invoker->push_updates_listener = NULL;
        }
    }
}

#undef X_CONF_FETCH_NOTIFICATIONS_PATH_BUFFER_SIZE

void ROX_INTERNAL x_configuration_fetched_handler(void *target, ConfigurationFetchedArgs *args) {
    assert(target);
    assert(args);
    if (args->fetcher_status != ErrorFetchedFailed) {
        _start_or_stop_push_updates_listener((XConfigurationFetchedInvoker *) target);
    }
}

void ROX_INTERNAL x_configuration_fetched_invoker_free(XConfigurationFetchedInvoker *invoker) {
    assert(invoker);
    free(invoker);
}
