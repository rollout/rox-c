#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "core/consts.h"
#include "core/logging.h"
#include "network.h"
#include "util.h"
#include "collections.h"

//
// Debouncer
//

struct Debouncer {
    int interval_millis;
    void *target;
    debouncer_func func;
    double cancel_until;
    pthread_t thread;
    pthread_mutex_t thread_mutex;
    pthread_cond_t thread_cond;
    bool thread_started;
    bool stopped;
};

ROX_INTERNAL Debouncer *debouncer_create(int interval_millis, void *target, debouncer_func func) {
    assert(interval_millis > 0);
    assert(func);
    Debouncer *debouncer = calloc(1, sizeof(Debouncer));
    debouncer->interval_millis = interval_millis;
    debouncer->target = target;
    debouncer->func = func;
    debouncer->cancel_until = current_time_millis();
    debouncer->thread_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    debouncer->thread_cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    ROX_TRACE("[%f] Initialize debouncer with interval %dms (cancel until %f)",
              current_time_millis(),
              interval_millis,
              debouncer->cancel_until);
    return debouncer;
}

static void *_debouncer_thread_func(void *ptr) {
    Debouncer *debouncer = (Debouncer *) ptr;
    if (!debouncer->stopped) {
        double time = current_time_millis();
        ROX_TRACE("[%f] in _debouncer_thread_func", current_time_millis());
        if (time >= debouncer->cancel_until) {
            ROX_TRACE("[%f] it's time", current_time_millis());
            // The following code is an analogue of sleep() with the exception
            // that it allows the thread to be awakened by the debouncer when
            // it's destroyed for example.
            struct timespec ts = get_future_timespec(debouncer->interval_millis);
            ROX_TRACE("[%f] waiting %dms before execute debouncer func", current_time_millis(),
                      debouncer->interval_millis);
            pthread_mutex_lock(&debouncer->thread_mutex);
            int result = pthread_cond_timedwait(&debouncer->thread_cond, &debouncer->thread_mutex, &ts);
            ROX_TRACE("[%f] timed wait done, result is %d", current_time_millis(), result);
            pthread_mutex_unlock(&debouncer->thread_mutex);
            if (result == ETIMEDOUT) {
                debouncer->func(debouncer->target);
            }
        }
    }
    debouncer->thread_started = false;
    return NULL;
}

ROX_INTERNAL void debouncer_invoke(Debouncer *debouncer) {
    assert(debouncer);
    if (!debouncer->thread_started) {
        debouncer->thread_started = (pthread_create(
                &debouncer->thread, NULL, _debouncer_thread_func, (void *) debouncer) == 0);
    }
}

ROX_INTERNAL void debouncer_free(Debouncer *debouncer) {
    assert(debouncer);
    debouncer->stopped = true;
    if (debouncer->thread_started) {
        pthread_cancel(debouncer->thread);
        pthread_join(debouncer->thread, NULL);
    }
    free(debouncer);
}

//
// StateSender
//

struct StateSender {
    Request *request;
    DeviceProperties *device_properties;
    FlagRepository *flag_repository;
    CustomPropertyRepository *custom_property_repository;
    Debouncer *state_debouncer;
    RoxList *state_generators;
    RoxList *relevant_api_call_params;
    RoxList *raw_json_params;
};

static int _state_sender_list_key_cmp(const void *e1, const void *e2) {
    const char *s1 = *(char **) e1;
    const char *s2 = *(char **) e2;
    return strcmp(s1, s2);
}

static RoxList *_state_sender_get_sorted_keys(RoxMap *map) {
    assert(map);
    RoxList *keys = rox_list_create();
    ROX_MAP_FOREACH(key, value, map, {
        rox_list_add(keys, key);
    })
    rox_list_sort(keys, &_state_sender_list_key_cmp);
    return keys;
}

static char *_state_sender_serialize_feature_flags(StateSender *sender) {
    assert(sender);
    cJSON *arr = cJSON_CreateArray();
    RoxMap *flags = flag_repository_get_all_flags(sender->flag_repository);
    RoxList *keys = _state_sender_get_sorted_keys(flags);
    ROX_LIST_FOREACH(key, keys, {
        RoxVariant *flag;
        if (rox_map_get(flags, key, (void **) &flag)) {
            cJSON *options_arr = cJSON_CreateArray();
            RoxListIter *list_iter = rox_list_iter_create();
            rox_list_iter_init(list_iter, variant_get_options(flag));
            char *option;
            while (rox_list_iter_next(list_iter, (void **) &option)) {
                cJSON_AddItemToArray(options_arr, ROX_JSON_STRING(option));
            }
            rox_list_iter_free(list_iter);
            const char *default_value = variant_get_default_value(flag);
            const char *variant_name = variant_get_name(flag);
            cJSON_AddItemToArray(arr, ROX_JSON_OBJECT(
                    "name", variant_name ? ROX_JSON_STRING(variant_name) : ROX_JSON_NULL,
                    "defaultValue", default_value ? ROX_JSON_STRING(default_value) : ROX_JSON_NULL,
                    "options", options_arr
            ));
        }
    })
    char *json_str = ROX_JSON_SERIALIZE(arr);
    cJSON_Delete(arr);
    rox_list_free(keys);
    return json_str;
}

static char *_state_sender_serialize_custom_properties(StateSender *sender) {
    assert(sender);
    cJSON *arr = cJSON_CreateArray();
    RoxMap *props = custom_property_repository_get_all_custom_properties(sender->custom_property_repository);

    RoxList *keys = _state_sender_get_sorted_keys(props);
    ROX_LIST_FOREACH(key, keys, {
        CustomProperty *property;
        if (rox_map_get(props, key, (void **) &property)) {
            cJSON_AddItemToArray(arr, custom_property_to_json(property));
        }
    })
    rox_list_free(keys);

    char *json_str = ROX_JSON_SERIALIZE(arr);
    cJSON_Delete(arr);
    return json_str;
}

static char *_state_sender_get_state_md5(StateSender *sender, RoxMap *properties) {
    assert(sender);
    assert(properties);
    return md5_generator_generate(properties, sender->state_generators, NULL);
}

static RoxMap *_state_sender_prepare_props_from_device_props(StateSender *sender) {
    assert(sender);
    RoxMap *properties = mem_deep_copy_str_value_map(
            device_properties_get_all_properties(sender->device_properties));
    rox_map_add(properties, ROX_PROPERTY_TYPE_FEATURE_FLAGS.name, _state_sender_serialize_feature_flags(sender));
    rox_map_add(properties, ROX_PROPERTY_TYPE_REMOTE_VARIABLES.name, mem_copy_str("[]"));
    rox_map_add(properties, ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES.name,
                _state_sender_serialize_custom_properties(sender));
    char *state_md5 = _state_sender_get_state_md5(sender, properties);
    rox_map_add(properties, ROX_PROPERTY_TYPE_STATE_MD5.name, state_md5);
    return properties;
}

static char *_state_sender_get_path(RoxMap *properties) {
    char *app_key, *state_md5;
    if (rox_map_get(properties, ROX_PROPERTY_TYPE_APP_KEY.name, (void **) &app_key) &&
        rox_map_get(properties, ROX_PROPERTY_TYPE_STATE_MD5.name, (void **) &state_md5)) {
        return mem_str_format("%s/%s", app_key, state_md5);
    }
    return NULL;
}

#define ROX_STATE_SENDER_URL_BUFFER_LENGTH 1024

static char *_state_sender_get_url(RoxMap *properties, size_t (*get_url_func)(char *, size_t)) {
    char *path = _state_sender_get_path(properties);
    if (!path) {
        return NULL;
    }
    char buffer[ROX_STATE_SENDER_URL_BUFFER_LENGTH];
    get_url_func(buffer, ROX_STATE_SENDER_URL_BUFFER_LENGTH);
    char *url = mem_str_format("%s/%s", buffer, path);
    free(path);
    return url;
}

#undef ROX_STATE_SENDER_URL_BUFFER_LENGTH

static char *_state_sender_get_cdn_url(RoxMap *properties) {
    return _state_sender_get_url(properties, &rox_env_get_state_cdn_path);
}

static char *_state_sender_get_api_url(RoxMap *properties) {
    return _state_sender_get_url(properties, &rox_env_get_state_api_path);
}

static HttpResponseMessage *_state_sender_send_state_to_cdn(StateSender *sender, RoxMap *properties) {
    char *url = _state_sender_get_cdn_url(properties);
    if (!url) {
        return NULL;
    }
    RequestData *cdn_request = request_data_create(url, NULL, NULL);
    HttpResponseMessage *response = request_send_get(sender->request, cdn_request);
    request_data_free(cdn_request);
    free(url);
    return response;
}

static HttpResponseMessage *_state_sender_send_state_to_api(StateSender *sender, RoxMap *properties) {
    char *url = _state_sender_get_api_url(properties);
    RoxMap *query_params = ROX_EMPTY_MAP;
    ROX_LIST_FOREACH(item, sender->relevant_api_call_params, {
        PropertyType *type = (PropertyType *) item;
        char *prop_name = type->name;
        char *prop_value = NULL;
        if (rox_map_get(properties, prop_name, (void **) &prop_value)) {
            rox_map_add(query_params, prop_name, prop_value);
        }
    })
    RequestData *api_request = request_data_create(url, query_params, sender->raw_json_params);
    HttpResponseMessage *response = request_send_post(sender->request, api_request);
    rox_map_free(query_params);
    request_data_free(api_request);
    free(url);
    return response;
}

static void _state_sender_log_log_send_state_exception(ConfigurationSource source) {
    ROX_ERROR("Failed to send state. Source: %s", configuration_source_to_str(source));
}

ROX_INTERNAL void state_sender_send(StateSender *sender) {
    assert(sender);

    RoxMap *properties = _state_sender_prepare_props_from_device_props(sender);
    bool should_retry = false;
    ConfigurationSource source = CONFIGURATION_SOURCE_CDN;

    HttpResponseMessage *fetch_result = _state_sender_send_state_to_cdn(sender, properties);
    if (!fetch_result) {
        _state_sender_log_log_send_state_exception(source);
        rox_map_free_with_values(properties);
        return;
    }

    if (response_message_is_successful(fetch_result)) {
        char *response_as_string = response_get_contents(fetch_result);
        cJSON *response_json = cJSON_Parse(response_as_string);
        if (response_json) {
            cJSON *result = cJSON_GetObjectItem(response_json, "result");
            if (result && (result->valueint == 404 ||
                           (result->valuestring && str_equals(result->valuestring, "404")))) {
                should_retry = true;
            }
        }

        if (!should_retry) {
            // success from cdn
            if (response_json) {
                cJSON_Delete(response_json);
                response_message_free(fetch_result);
                rox_map_free_with_values(properties);
                return;
            }
        }

        if (response_json) {
            cJSON_Delete(response_json);
        }
    }

    if (should_retry ||
        response_message_get_status(fetch_result) == 403 ||
        response_message_get_status(fetch_result) == 404) {

        ROX_DEBUG("Failed to send state to %s. Trying to send state to %s. http result code: %d",
                  configuration_source_to_str(CONFIGURATION_SOURCE_CDN),
                  configuration_source_to_str(CONFIGURATION_SOURCE_API),
                  response_message_get_status(fetch_result));

        source = CONFIGURATION_SOURCE_API;

        response_message_free(fetch_result);
        fetch_result = _state_sender_send_state_to_api(sender, properties);

        if (!fetch_result) {
            _state_sender_log_log_send_state_exception(source);
            rox_map_free_with_values(properties);
            return;
        }

        if (response_message_is_successful(fetch_result)) {
            // success for api
            response_message_free(fetch_result);
            rox_map_free_with_values(properties);
            return;
        }

    }

    ROX_ERROR("Failed to send state. Source: %s (%s)",
              configuration_source_to_str(source),
              fetch_result
              ? response_get_contents(fetch_result)
              : "unknown error");

    response_message_free(fetch_result);
    rox_map_free_with_values(properties);
}

ROX_INTERNAL void state_sender_send_debounce(StateSender *sender) {
    assert(sender);
    debouncer_invoke(sender->state_debouncer);
}

static void _state_sender_custom_property_handler(void *target, CustomProperty *property) {
    assert(target);
    assert(property);
    StateSender *sender = (StateSender *) target;
    state_sender_send_debounce(sender);
}

static void _state_sender_flag_added_callback(void *target, RoxVariant *variant) {
    assert(target);
    assert(variant);
    StateSender *sender = (StateSender *) target;
    state_sender_send_debounce(sender);
}

ROX_INTERNAL StateSender *state_sender_create(
        Request *request,
        DeviceProperties *device_properties,
        FlagRepository *flag_repository,
        CustomPropertyRepository *custom_property_repository) {

    assert(request);
    assert(device_properties);
    assert(flag_repository);
    assert(custom_property_repository);

    StateSender *sender = calloc(1, sizeof(StateSender));
    sender->request = request;
    sender->device_properties = device_properties;
    sender->flag_repository = flag_repository;
    sender->custom_property_repository = custom_property_repository;
    sender->state_debouncer = debouncer_create(3000, sender, (debouncer_func) &state_sender_send);

    sender->state_generators = ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_APP_KEY,
            &ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES,
            &ROX_PROPERTY_TYPE_FEATURE_FLAGS,
            &ROX_PROPERTY_TYPE_REMOTE_VARIABLES,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET);

    sender->relevant_api_call_params = ROX_LIST(
            &ROX_PROPERTY_TYPE_PLATFORM,
            &ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES,
            &ROX_PROPERTY_TYPE_FEATURE_FLAGS,
            &ROX_PROPERTY_TYPE_REMOTE_VARIABLES,
            &ROX_PROPERTY_TYPE_DEV_MODE_SECRET);

    sender->raw_json_params = ROX_LIST(
            ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES.name,
            ROX_PROPERTY_TYPE_FEATURE_FLAGS.name,
            ROX_PROPERTY_TYPE_REMOTE_VARIABLES.name);

    custom_property_repository_set_handler(custom_property_repository, sender, &_state_sender_custom_property_handler);
    flag_repository_add_flag_added_callback(flag_repository, sender, &_state_sender_flag_added_callback);

    return sender;
}

ROX_INTERNAL void state_sender_free(StateSender *sender) {
    assert(sender);
    debouncer_free(sender->state_debouncer);
    rox_list_free(sender->state_generators);
    rox_list_free(sender->relevant_api_call_params);
    rox_list_free(sender->raw_json_params);
    free(sender);
}
