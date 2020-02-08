#pragma once

#include <collectc/hashtable.h>
#include <cjson/cJSON.h>
#include "roxapi.h"
#include "properties.h"
#include "client.h"
#include "configuration.h"
#include "reporting.h"

//
// RequestData
//

typedef struct ROX_INTERNAL RequestData {
    char *url;
    HashTable *params;
} RequestData;

/**
 * @param url Not <code>NULL</code>. Data is copied internally, the caller holds an ownership on the data.
 * @param params May be <code>NULL</code>. If passed, ownership is NOT delegated to the returned <code>RequestData</code> so the caller is responsible for freeing it.
 * @return Not <code>NULL</code>.
 */
RequestData *ROX_INTERNAL request_data_create(const char *url, HashTable *params);

/**
 * @param data Not <code>NULL</code>.
 */
void ROX_INTERNAL request_data_free(RequestData *data);

//
// HttpResponseMessage
//

typedef struct ROX_INTERNAL HttpResponseMessage HttpResponseMessage;

/**
 * @param status >= 0
 * @param data May be <code>NULL</code>. If passed, the ownership is delegated to the returned
 * <code>HttpResponseMessage</code> and the data will be destroyed when calling <code>response_message_free()</code>.
 * @return Not <code>NULL</code>.
 */
HttpResponseMessage *ROX_INTERNAL response_message_create(int status, char *data);

/**
 * @param message Not <code>NULL</code>.
 */
int ROX_INTERNAL response_message_get_status(HttpResponseMessage *message);

/**
 * @param message Not <code>NULL</code>.
 */
bool ROX_INTERNAL response_message_is_successful(HttpResponseMessage *message);

/**
 * Note: the returned string must be freed after use by the caller.
 * @param message Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
char *ROX_INTERNAL response_read_as_string(HttpResponseMessage *message);

/**
 * @param message Not <code>NULL</code>.
 */
void ROX_INTERNAL response_message_free(HttpResponseMessage *message);

//
// Request
//

typedef struct ROX_INTERNAL Request Request;

typedef HttpResponseMessage *(*request_send_get_func)(void *target, Request *request, RequestData *data);

typedef HttpResponseMessage *(*request_send_post_func)(void *target, Request *request, RequestData *data);

typedef HttpResponseMessage *(*request_send_post_json_func)(void *target, Request *request, const char *uri,
                                                            cJSON *json);

/**
 * The following is needed basically for unit testing.
 */
typedef struct ROX_INTERNAL RequestConfig {
    void *target;
    request_send_get_func send_get;
    request_send_post_func send_post;
    request_send_post_json_func send_post_json;
    int request_timeout;
} RequestConfig;

/**
 * @param config May be <code>NULL</code>. Clients are responsible for freeing the memory.
 * @return Not <code>NULL</code>.
 */
Request *ROX_INTERNAL request_create(RequestConfig *config);

/**
 * @param request Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
HttpResponseMessage *ROX_INTERNAL request_send_get(Request *request, RequestData *data);

/**
 * @param request Not <code>NULL</code>.
 * @param uri Not <code>NULL</code>.
 * @param json Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
HttpResponseMessage *ROX_INTERNAL request_send_post_json(Request *request, const char *uri, cJSON *json);

/**
 * @param request Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
HttpResponseMessage *ROX_INTERNAL request_send_post(Request *request, RequestData *data);

/**
 * @param request Not <code>NULL</code>.
 */
void ROX_INTERNAL request_free(Request *request);

//
// ConfigurationFetcher
//

typedef struct ROX_INTERNAL ConfigurationFetcher ConfigurationFetcher;

/**
 * @param request Not <code>NULL</code>.
 * @param sdk_settings Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @param invoker Not <code>NULL</code>.
 * @param reporter Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ConfigurationFetcher *ROX_INTERNAL configuration_fetcher_create(
        Request *request,
        SdkSettings *sdk_settings,
        DeviceProperties *device_properties,
        BUID *buid,
        ConfigurationFetchedInvoker *invoker,
        ErrorReporter *reporter);

/**
 * @param request Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @param invoker Not <code>NULL</code>.
 * @param reporter Not <code>NULL</code>.
 * @param roxy_url Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ConfigurationFetcher *ROX_INTERNAL configuration_fetcher_create_roxy(
        Request *request,
        DeviceProperties *device_properties,
        BUID *buid,
        ConfigurationFetchedInvoker *invoker,
        ErrorReporter *reporter,
        const char *roxy_url);

/**
 * @param fetcher Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ConfigurationFetchResult *ROX_INTERNAL configuration_fetcher_fetch(ConfigurationFetcher *fetcher);

/**
 * @param fetcher Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetcher_free(ConfigurationFetcher *fetcher);
