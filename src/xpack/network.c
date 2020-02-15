#include <collectc/hashtable.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "core/consts.h"
#include "core/logging.h"
#include "roxx/stack.h"
#include "network.h"
#include "util.h"

//
// Debouncer
//

struct ROX_INTERNAL Debouncer {
    int interval_millis;
    void *target;
    debouncer_func func;
    double cancel_until;
    pthread_t thread;
    pthread_mutex_t thread_mutex;
    pthread_cond_t thread_cond;
    bool thread_started;
};

Debouncer *ROX_INTERNAL debouncer_create(int interval_millis, void *target, debouncer_func func) {
    assert(interval_millis > 0);
    assert(func);
    Debouncer *debouncer = calloc(1, sizeof(Debouncer));
    debouncer->interval_millis = interval_millis;
    debouncer->target = target;
    debouncer->func = func;
    debouncer->cancel_until = current_time_millis();
    debouncer->thread_mutex = PTHREAD_MUTEX_INITIALIZER;
    debouncer->thread_cond = PTHREAD_COND_INITIALIZER;
    return debouncer;
}

static void *_debouncer_thread_func(void *ptr) {
    Debouncer *debouncer = (Debouncer *) ptr;
    double time = current_time_millis();
    if (time >= debouncer->cancel_until) {
        // The following code is an analogue of sleep() with the exception
        // that it allows the thread to be awakened by the debouncer when
        // it's destroyed for example.
        struct timespec ts = get_future_timespec(debouncer->interval_millis);
        pthread_mutex_lock(&debouncer->thread_mutex);
        int result = pthread_cond_timedwait(&debouncer->thread_cond, &debouncer->thread_mutex, &ts);
        pthread_mutex_unlock(&debouncer->thread_mutex);
        if (result == ETIMEDOUT) {
            debouncer->func(debouncer->target);
        }
    }
    debouncer->thread_started = false;
    return NULL;
}

void ROX_INTERNAL debouncer_invoke(Debouncer *debouncer) {
    assert(debouncer);
    if (!debouncer->thread_started) {
        debouncer->thread_started = (pthread_create(
                &debouncer->thread, NULL, _debouncer_thread_func, (void *) debouncer) == 0);
    }
}

void ROX_INTERNAL debouncer_free(Debouncer *debouncer) {
    assert(debouncer);
    if (debouncer->thread_started) {
        pthread_mutex_lock(&debouncer->thread_mutex);
        pthread_cond_signal(&debouncer->thread_cond);
        pthread_mutex_unlock(&debouncer->thread_mutex);
        // wait for thread to finish
        // (it should finish immediately because we sent signal to its condition)
        pthread_join(debouncer->thread, NULL);
    }
    assert(!debouncer->thread_started);
    free(debouncer);
}

//
// StateSender
//

struct ROX_INTERNAL StateSender {
    Request *request;
    DeviceProperties *device_properties;
    FlagRepository *flag_repository;
    CustomPropertyRepository *custom_property_repository;
    Debouncer *state_debouncer;
    List *state_generators;
    List *relevant_api_call_params;
};

static char *_state_sender_serialize_feature_flags(StateSender *sender) {
    assert(sender);
    cJSON *arr = cJSON_CreateObject();
    HashTable *flags = flag_repository_get_all_flags(sender->flag_repository);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, flags, {
        Variant *flag = (Variant *) entry->value;
        cJSON *options_arr = cJSON_CreateArray();
        LIST_FOREACH(item, flag->options, {
                char* option = (char*)item;
                cJSON_AddItemToArray(item, ROX_JSON_STRING(option));
        })
        cJSON_AddItemToArray(arr, ROX_JSON_OBJECT(
                "name", flag->name,
                "defaultValue", flag->default_value,
                "options", options_arr
        ));
    })
    char *json_str = ROX_JSON_SERIALIZE(arr);
    cJSON_Delete(arr);
    return json_str;
}

static char *_state_sender_serialize_custom_properties(StateSender *sender) {
    assert(sender);
    cJSON *arr = cJSON_CreateArray();
    HashTable *props = custom_property_repository_get_all_custom_properties(sender->custom_property_repository);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, props, {
        CustomProperty *property = (CustomProperty *) entry->value;
        cJSON_AddItemToArray(arr, custom_property_to_json(property));
    })
    char *json_str = ROX_JSON_SERIALIZE(arr);
    cJSON_Delete(arr);
    return json_str;
}

static char *_state_sender_get_state_md5(StateSender *sender, HashTable *properties) {
    assert(sender);
    assert(properties);
    return md5_generator_generate(properties, sender->state_generators, NULL);
}

static HashTable *_state_sender_prepare_props_from_device_props(StateSender *sender) {
    assert(sender);
    HashTable *properties = mem_deep_copy_str_value_map(
            device_properties_get_all_properties(sender->device_properties));
    hashtable_add(properties, ROX_PROPERTY_TYPE_FEATURE_FLAGS.name, _state_sender_serialize_feature_flags(sender));
    hashtable_add(properties, ROX_PROPERTY_TYPE_REMOTE_VARIABLES.name, ROX_JSON_SERIALIZE(ROX_EMPTY_JSON_ARRAY));
    hashtable_add(properties, ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES.name,
                  _state_sender_serialize_custom_properties(sender));
    char *state_md5 = _state_sender_get_state_md5(sender, properties);
    hashtable_add(properties, ROX_PROPERTY_TYPE_STATE_MD5.name, state_md5);
    return properties;
}

static char *_state_sender_get_path(HashTable *properties) {
    char *app_key, *state_md5;
    if (hashtable_get(properties, ROX_PROPERTY_TYPE_APP_KEY.name, (void **) &app_key) == CC_OK &&
        hashtable_get(properties, ROX_PROPERTY_TYPE_STATE_MD5.name, (void **) &state_md5) == CC_OK) {
        return mem_str_format("%s/%s", app_key, state_md5);
    }
    return NULL;
}

#define ROX_STATE_SENDER_URL_BUFFER_LENGTH 1024

static char *_state_sender_get_url(HashTable *properties, size_t (*get_url_func)(char *, size_t)) {
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

static char *_state_sender_get_cdn_url(HashTable *properties) {
    return _state_sender_get_url(properties, &rox_env_get_state_cdn_path);
}

static char *_state_sender_get_api_url(HashTable *properties) {
    return _state_sender_get_url(properties, &rox_env_get_state_api_path);
}

static HttpResponseMessage *_state_sender_send_state_to_cdn(StateSender *sender, HashTable *properties) {
    char *url = _state_sender_get_cdn_url(properties);
    if (!url) {
        return NULL;
    }
    RequestData *cdn_request = request_data_create(url, NULL);
    HttpResponseMessage *response = request_send_get(sender->request, cdn_request);
    free(url);
    request_data_free(cdn_request);
    return response;
}

static HttpResponseMessage *_state_sender_send_state_to_api(StateSender *sender, HashTable *properties) {
    char *url = _state_sender_get_api_url(properties);
    HashTable *query_params = ROX_EMPTY_MAP;
    LIST_FOREACH(item, sender->relevant_api_call_params, {
        PropertyType *type = (PropertyType *) item;
        char *prop_name = type->name;
        char *prop_value = NULL;
        if (CC_OK == hashtable_get(properties, prop_name, (void **) &prop_value)) {
            hashtable_add(query_params, prop_name, prop_value);
        }
    })
    RequestData *api_request = request_data_create(url, query_params);
    HttpResponseMessage *response = request_send_post(sender->request, api_request);
    hashtable_destroy(query_params);
    return response;
}

static void _state_sender_log_send_state_error(
        ConfigurationSource source,
        HttpResponseMessage *response,
        ConfigurationSource next_source) {
    if (next_source) {
        ROX_DEBUG("Failed to send state to %d. Trying to send state to %d. http result code: %d",
                  source, response_message_get_status(response), next_source);
    } else {
        ROX_DEBUG("Failed to send state to %d. http result code: %d",
                  source, response_message_get_status(response));
    }
}

static void _state_sender_log_log_send_state_exception(ConfigurationSource source) {
    ROX_ERROR("Failed to send state. Source: %d", source);
}

void ROX_INTERNAL state_sender_send(StateSender *sender) {
    assert(sender);

    HashTable *properties = _state_sender_prepare_props_from_device_props(sender);
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
            if (result && result->valueint == 404) {
                should_retry = true;
            }
        }

        if (!should_retry) {
            // success from cdn
            response_message_free(fetch_result);
            rox_map_free_with_values(properties);
            return;
        }
    }

    if (should_retry ||
        response_message_get_status(fetch_result) == 403 ||
        response_message_get_status(fetch_result) == 404) {
        _state_sender_log_send_state_error(source, fetch_result, CONFIGURATION_SOURCE_API);
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

    _state_sender_log_send_state_error(source, fetch_result, 0);
}

static void _state_sender_custom_property_handler(void *target, CustomProperty *property) {
    assert(target);
    assert(property);
    StateSender *sender = (StateSender *) target;
    debouncer_invoke(sender->state_debouncer);
}

static void _state_sender_flag_added_callback(void *target, Variant *variant) {
    assert(target);
    assert(variant);
    StateSender *sender = (StateSender *) target;
    debouncer_invoke(sender->state_debouncer);
}

StateSender *ROX_INTERNAL state_sender_create(
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

    custom_property_repository_set_handler(custom_property_repository, sender, &_state_sender_custom_property_handler);
    flag_repository_add_flag_added_callback(flag_repository, sender, &_state_sender_flag_added_callback);

    return sender;
}

void ROX_INTERNAL state_sender_free(StateSender *sender) {
    assert(sender);
    list_destroy(sender->state_generators);
    list_destroy(sender->relevant_api_call_params);
    debouncer_free(sender->state_debouncer);
    free(sender);
}
