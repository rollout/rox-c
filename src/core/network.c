#include <collectc/list.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <assert.h>
#include "network.h"
#include "util.h"
#include "consts.h"

//
// RequestData
//

RequestData *ROX_INTERNAL request_data_create(const char *url, HashTable *params) {
    assert(url);
    RequestData *data = calloc(1, sizeof(RequestData));
    data->url = mem_copy_str(url);
    data->params = params;
    return data;
}

void ROX_INTERNAL request_data_free(RequestData *data) {
    assert(data);
    free(data->url);
    free(data);
}

//
// HttpResponseMessage
//

struct ROX_INTERNAL HttpResponseMessage {
    int status;
    char *content;
};

HttpResponseMessage *ROX_INTERNAL response_message_create(int status, char *data) {
    assert(status >= 0);
    HttpResponseMessage *message = calloc(1, sizeof(HttpResponseMessage));
    message->status = status;
    message->content = data;
    return message;
}

int ROX_INTERNAL response_message_get_status(HttpResponseMessage *message) {
    assert(message);
    return message->status;
}

bool ROX_INTERNAL response_message_is_successful(HttpResponseMessage *message) {
    assert(message);
    return message->status >= 200 && message->status <= 299;
}

void ROX_INTERNAL response_message_free(HttpResponseMessage *message) {
    assert(message);
    if (message->content) {
        free(message->content);
    }
    free(message);
}

char *ROX_INTERNAL response_read_as_string(HttpResponseMessage *message) {
    assert(message);
    return message->content;
}

//
// Request
//

struct ROX_INTERNAL Request {
    void *target;
    request_send_get_func send_get;
    request_send_post_func send_post;
    request_send_post_json_func send_post_json;
    CURL *curl;
};

typedef struct ROX_INTERNAL RequestCurlContext {
    Request *request;
    HttpResponseMessage *message;
} RequestCurlContext;

static size_t _request_curl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t real_size = size * nmemb;
    RequestCurlContext *context = (RequestCurlContext *) userdata;
    HttpResponseMessage *message = context->message;
    if (message->content) {
        free(message->content);
    }
    message->content = malloc(real_size + 1);
    memcpy(message->content, ptr, real_size);
    message->content[real_size] = 0;
    return real_size;
}

static char *_request_build_url_with_params(Request *request, const char *url, HashTable *params) {
    assert(request);
    assert(url);
    assert(params);
    List *kv_pairs;
    list_new(&kv_pairs);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, params, {
        char *key = curl_easy_escape(request->curl, entry->key, 0);
        char *value = curl_easy_escape(request->curl, entry->value, 0);
        char *pair = mem_str_format("%s=%s", key, value);
        free(key);
        free(value);
        list_add(kv_pairs, pair);
    })
    char *query_part = mem_str_join("&", kv_pairs);
    list_destroy_cb(kv_pairs, &free);
    char *result = mem_str_format("%s?%s", url, query_part);
    free(query_part);
    return result;
}

static cJSON *_build_json_from_params(HashTable *params) {
    assert(params);
    cJSON *json = cJSON_CreateObject();
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, params, {
        cJSON *item = cJSON_CreateString(entry->value);
        cJSON_AddItemToObject(json, entry->key, item);
    })
    return json;
}

static HttpResponseMessage *_request_send_get(void *target, Request *request, RequestData *data) {
    assert(request);
    assert(data);
    char *url = data->params
                ? _request_build_url_with_params(request, data->url, data->params)
                : data->url;
    HttpResponseMessage *message = response_message_create(0, NULL);
    RequestCurlContext context = {request, message};
    curl_easy_setopt(request->curl, CURLOPT_URL, url);
    curl_easy_setopt(request->curl, CURLOPT_HTTPGET, true);
    curl_easy_setopt(request->curl, CURLOPT_WRITEDATA, &context);
    CURLcode res = curl_easy_perform(request->curl);
    if (res != CURLE_OK) {
        // TODO: log
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    if (url != data->url) {
        free(url);
    }
    return message;
}

static HttpResponseMessage *_request_send_post_json(void *target, Request *request, const char *uri, cJSON *json) {
    assert(request);
    assert(uri);
    assert(json);

    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");

    HttpResponseMessage *message = response_message_create(0, NULL);
    RequestCurlContext context = {request, message};
    curl_easy_setopt(request->curl, CURLOPT_URL, uri);
    curl_easy_setopt(request->curl, CURLOPT_HTTPPOST, true);
    curl_easy_setopt(request->curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(request->curl, CURLOPT_WRITEDATA, &context);
    curl_easy_setopt(request->curl, CURLOPT_HTTPHEADER, headers);
    CURLcode res = curl_easy_perform(request->curl);
    curl_slist_free_all(headers);
    if (res != CURLE_OK) {
        // TODO: log
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
    free(json);
    return message;
}

static HttpResponseMessage *_request_send_post(void *target, Request *request, RequestData *data) {
    assert(request);
    assert(data);
    assert(data->params);
    cJSON *json = _build_json_from_params(data->params);
    HttpResponseMessage *message = _request_send_post_json(target, request, data->url, json);
    cJSON_Delete(json);
    return message;
}

Request *ROX_INTERNAL request_create(RequestConfig *config) {
    Request *request = calloc(1, sizeof(Request));
    if (config) {
        request->target = config->target;
        request->send_get = config->send_get;
        request->send_post = config->send_post;
        request->send_post_json = config->send_post_json;
    }
    if (!request->send_get) {
        request->send_get = &_request_send_get;
    }
    if (!request->send_post) {
        request->send_post = &_request_send_post;
    }
    if (!request->send_post_json) {
        request->send_post_json = &_request_send_post_json;
    }
    request->curl = curl_easy_init();
    curl_easy_setopt(request->curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(request->curl, CURLOPT_ACCEPT_ENCODING, ""); // enable all supported built-in compressions
    curl_easy_setopt(request->curl, CURLOPT_WRITEFUNCTION, &_request_curl_write_callback);
    curl_easy_setopt(request->curl, CURLOPT_TIMEOUT, config && config->request_timeout > 0
                                                     ? config->request_timeout : 30);
    return request;
}

HttpResponseMessage *ROX_INTERNAL request_send_get(Request *request, RequestData *data) {
    assert(request);
    assert(data);
    return request->send_get(request->target, request, data);
}

HttpResponseMessage *ROX_INTERNAL request_send_post(Request *request, RequestData *data) {
    assert(request);
    assert(data);
    return request->send_post(request->target, request, data);
}

HttpResponseMessage *ROX_INTERNAL request_send_post_json(Request *request, const char *uri, cJSON *json) {
    assert(request);
    assert(uri);
    assert(json);
    return request->send_post_json(request->target, request, uri, json);
}

void ROX_INTERNAL request_free(Request *request) {
    assert(request);
    curl_easy_cleanup(request->curl);
    free(request);
}

//
// ConfigurationFetcher
//

struct ROX_INTERNAL ConfigurationFetcher {
    Request *request;
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    BUID *buid;
    ConfigurationFetchedInvoker *invoker;
    ErrorReporter *reporter;
    char *roxy_url;
};

ConfigurationFetcher *ROX_INTERNAL configuration_fetcher_create(
        Request *request,
        SdkSettings *sdk_settings,
        DeviceProperties *device_properties,
        BUID *buid,
        ConfigurationFetchedInvoker *invoker,
        ErrorReporter *reporter) {
    assert(request);
    assert(sdk_settings);
    assert(device_properties);
    assert(buid);
    assert(invoker);
    assert(reporter);
    ConfigurationFetcher *fetcher = calloc(1, sizeof(ConfigurationFetcher));
    fetcher->request = request;
    fetcher->sdk_settings = sdk_settings;
    fetcher->device_properties = device_properties;
    fetcher->buid = buid;
    fetcher->invoker = invoker;
    fetcher->reporter = reporter;
    return fetcher;
}

ConfigurationFetcher *ROX_INTERNAL configuration_fetcher_create_roxy(
        Request *request,
        DeviceProperties *device_properties,
        BUID *buid,
        ConfigurationFetchedInvoker *invoker,
        ErrorReporter *reporter,
        const char *roxy_url) {
    assert(request);
    assert(device_properties);
    assert(buid);
    assert(invoker);
    assert(reporter);
    assert(roxy_url);
    ConfigurationFetcher *fetcher = calloc(1, sizeof(ConfigurationFetcher));
    fetcher->request = request;
    fetcher->device_properties = device_properties;
    fetcher->buid = buid;
    fetcher->invoker = invoker;
    fetcher->reporter = reporter;
    fetcher->roxy_url = mem_copy_str(roxy_url);
    return fetcher;
}

static void _configuration_fetcher_handle_error(
        ConfigurationFetcher *fetcher,
        ConfigurationSource source,
        HttpResponseMessage *message,
        bool raiseConfigurationHandler,
        ConfigurationSource nextSource) {

    assert(fetcher);
    assert(source);

    char *retryMsg = NULL;
    if (nextSource) {
        retryMsg = mem_str_format("Trying from {0}. ", nextSource);
    }

// TODO: log
//Logging.Logging.GetLogger().Debug(
//        string.Format("Failed to fetch from {0}. {1}http error code: {2}", source, retryMsg, response.StatusCode));

    if (raiseConfigurationHandler) {
        configuration_fetched_invoker_invoke_error(fetcher->invoker, NetworkError);
    }
}

static ConfigurationFetchResult *_configuration_fetcher_create_result(
        ConfigurationFetcher *fetcher,
        HttpResponseMessage *message,
        ConfigurationSource source) {

    assert(fetcher);
    assert(message);

    char *data = message->content;
    if (str_is_empty(data)) {
        configuration_fetched_invoker_invoke_error(fetcher->invoker, EmptyJson);
        // TODO: log
//        Logging.Logging.GetLogger().Debug("Failed to parse JSON configuration - Null Or Empty", ae);
        error_reporter_report(fetcher->reporter, __FILE__, __LINE__, "Failed to parse JSON configuration - Null Or Empty");
        return NULL;
    }

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        configuration_fetched_invoker_invoke_error(fetcher->invoker, CorruptedJson);
        // TODO: log
//        Logging.Logging.GetLogger().Debug("Failed to parse JSON configuration", ex);
        error_reporter_report(fetcher->reporter, __FILE__, __LINE__, "Failed to parse JSON configuration");
        return NULL;
    }

    return configuration_fetch_result_create(json, source);
}

#define ROXY_URL_BUFFER_SIZE 1024

static HttpResponseMessage *_configuration_fetcher_internal_fetch(ConfigurationFetcher *fetcher) {
    assert(fetcher);
    char buffer[ROXY_URL_BUFFER_SIZE];
    rox_env_get_internal_path(buffer, ROXY_URL_BUFFER_SIZE);

    HashTable *params;
    hashtable_new(&params);
    HashTable *device_props = device_properties_get_all_properties(fetcher->device_properties);

    TableEntry *entry;
    HASHTABLE_FOREACH(entry, device_props, {
        hashtable_add(params, entry->key, entry->value);
    })

    RequestData *roxy_request = request_data_create(buffer, params);
    HttpResponseMessage *message = request_send_get(fetcher->request, roxy_request);
    request_data_free(roxy_request);
    hashtable_destroy(params);
    return message;
}

#undef ROXY_URL_BUFFER_SIZE

static ConfigurationFetchResult *_configuration_fetcher_fetch_using_roxy_url(ConfigurationFetcher *fetcher) {
    assert(fetcher);
    HttpResponseMessage *message = _configuration_fetcher_internal_fetch(fetcher);
    if (message && response_message_is_successful(message)) {
        return _configuration_fetcher_create_result(fetcher, message, CONFIGURATION_SOURCE_ROXY);
    } else {
        _configuration_fetcher_handle_error(fetcher, CONFIGURATION_SOURCE_ROXY, message, true, 0);
        return NULL;
    }
}

static HashTable *_configuration_fetcher_prepare_props_from_device_props(ConfigurationFetcher *fetcher) {
    assert(fetcher);
    HashTable *device_props = device_properties_get_all_properties(fetcher->device_properties);
    HashTable *params = mem_deep_copy_str_value_map(device_props);
    if (!hashtable_contains_key(params, ROX_PROPERTY_TYPE_BUID.name)) {
        char *buid = buid_get_value(fetcher->buid);
        hashtable_add(params, ROX_PROPERTY_TYPE_BUID.name, mem_copy_str(buid));
    }
    char *buid, *app_key;
    if (hashtable_get(params, ROX_PROPERTY_TYPE_BUID.name, (void **) &buid) == CC_OK &&
        hashtable_get(params, ROX_PROPERTY_TYPE_APP_KEY.name, (void **) &app_key) == CC_OK) {
        char *path = mem_str_format("%s/%s", app_key, buid);
        hashtable_add(params, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name, path);
    }
    return params;
}

#define ROX_FETCH_URL_BUFFER_SIZE 1024

HttpResponseMessage *ROX_INTERNAL configuration_fetcher_fetch_from_cdn(
        ConfigurationFetcher *fetcher,
        HashTable *properties) {
    assert(fetcher);
    assert(properties);
    char buffer[ROX_FETCH_URL_BUFFER_SIZE];
    rox_env_get_cdn_path(buffer, ROX_FETCH_URL_BUFFER_SIZE);
    char *path, *distinct_id;
    if (hashtable_get(properties, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name, (void **) &path) != CC_OK ||
        hashtable_get(properties, ROX_PROPERTY_TYPE_DISTINCT_ID.name, (void **) &distinct_id) != CC_OK) {
        return NULL;
    }
    char *url = mem_str_format("%s/%s", buffer, path);
    HashTable *params = ROX_MAP(ROX_PROPERTY_TYPE_DISTINCT_ID.name, distinct_id);
    RequestData *cdn_request = request_data_create(url, params);
    HttpResponseMessage *message = request_send_get(fetcher->request, cdn_request);
    request_data_free(cdn_request);
    hashtable_destroy(params);
    free(url);
    return message;
}

HttpResponseMessage *ROX_INTERNAL configuration_fetcher_fetch_from_api(
        ConfigurationFetcher *fetcher,
        HashTable *properties) {
    assert(fetcher);
    assert(properties);
    char buffer[ROX_FETCH_URL_BUFFER_SIZE];
    rox_env_get_api_path(buffer, ROX_FETCH_URL_BUFFER_SIZE);
    char *path;
    if (hashtable_get(properties, ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL.name, (void **) &path) != CC_OK) {
        return NULL;
    }
    char *url = mem_str_format("%s/%s", buffer, path);
    RequestData *api_request = request_data_create(url, properties);
    HttpResponseMessage *message = request_send_post(fetcher->request, api_request);
    request_data_free(api_request);
    free(url);
    return message;
}

#undef ROX_FETCH_URL_BUFFER_SIZE

ConfigurationFetchResult *ROX_INTERNAL configuration_fetcher_fetch(ConfigurationFetcher *fetcher) {
    assert(fetcher);
    if (fetcher->roxy_url) {
        return _configuration_fetcher_fetch_using_roxy_url(fetcher);
    }

    bool should_retry = false;
    ConfigurationSource source = CONFIGURATION_SOURCE_CDN;
    ConfigurationFetchResult *result = NULL;
    HashTable *properties = _configuration_fetcher_prepare_props_from_device_props(fetcher);
    HttpResponseMessage *message = configuration_fetcher_fetch_from_cdn(fetcher, properties);

    if (!message) {
        _configuration_fetcher_handle_error(fetcher, source, NULL, true, 0);
        rox_map_free_with_values(properties);
        return NULL;
    }

    if (response_message_is_successful(message)) {

        result = _configuration_fetcher_create_result(fetcher, message, source);

        if (!result) {
            rox_map_free_with_values(properties);
            response_message_free(message);
            return NULL;
        }

        if (!result->parsed_data) {
            rox_map_free_with_values(properties);
            response_message_free(message);
            configuration_fetch_result_free(result);
            return NULL;
        }

        cJSON *json_result = cJSON_GetObjectItem(result->parsed_data, "result");
        if (json_result && (json_result->valueint == 404 ||
                            (json_result->valuestring && str_equals(json_result->valuestring, "404")))) {
            configuration_fetch_result_free(result);
            should_retry = true;
        }

        if (!should_retry) {
            // success from cdn
            rox_map_free_with_values(properties);
            response_message_free(message);
            return result;
        }
    }

    if (should_retry || message->status == 403 || message->status == 404) {
        _configuration_fetcher_handle_error(fetcher, source, message, false, CONFIGURATION_SOURCE_API);
        source = CONFIGURATION_SOURCE_API;
        response_message_free(message);
        message = configuration_fetcher_fetch_from_api(fetcher, properties);
        if (!message) {
            _configuration_fetcher_handle_error(fetcher, source, NULL, true, 0);
            rox_map_free_with_values(properties);
            return NULL;
        }
        if (response_message_is_successful(message)) {
            rox_map_free_with_values(properties);
            return _configuration_fetcher_create_result(fetcher, message, source);
        }
    }

    rox_map_free_with_values(properties);
    response_message_free(message);
    _configuration_fetcher_handle_error(fetcher, source, NULL, true, 0);

    return NULL;
}

void ROX_INTERNAL configuration_fetcher_free(ConfigurationFetcher *fetcher) {
    assert(fetcher);
    if (fetcher->roxy_url) {
        free(fetcher->roxy_url);
    }
    free(fetcher);
}
