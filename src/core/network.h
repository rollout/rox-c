#pragma once

#include <cjson/cJSON.h>
#include "rox/defs.h"
#include "properties.h"
#include "client.h"
#include "configuration.h"
#include "reporting.h"
#include "collections.h"

//
// RequestData
//

typedef struct RequestData {
    char *url;
    RoxMap *params;
    RoxList *raw_json_params;
} RequestData;

/**
 * @param url Not <code>NULL</code>. Data is copied internally, the caller holds an ownership on the data.
 * @param params May be <code>NULL</code>. If passed, ownership is NOT delegated to the returned <code>RequestData</code> so the caller is responsible for freeing it.
 * @param raw_json_params May be <code>NULL</code>. List of parameter names whose values are already serialized JSON strings. If passed, ownership is <em>NOT</em> delegated to the request data.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RequestData *request_data_create(const char *url, RoxMap *params, RoxList *raw_json_params);

/**
 * @param data Not <code>NULL</code>.
 */
ROX_INTERNAL void request_data_free(RequestData *data);

//
// HttpResponseMessage
//

typedef struct HttpResponseMessage HttpResponseMessage;

/**
 * @param status >= 0
 * @param data May be <code>NULL</code>. If passed, the ownership is delegated to the returned
 * <code>HttpResponseMessage</code> and the data will be destroyed when calling <code>response_message_free()</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL HttpResponseMessage *response_message_create(int status, char *data);

/**
 * @param message Not <code>NULL</code>.
 */
ROX_INTERNAL int response_message_get_status(HttpResponseMessage *message);

/**
 * @param message Not <code>NULL</code>.
 */
ROX_INTERNAL bool response_message_is_successful(HttpResponseMessage *message);

/**
 * Note: the returned string must <em>NOT</em> be freed after use by the caller.
 * @param message Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL char *response_get_contents(HttpResponseMessage *message);

/**
 * @param message Not <code>NULL</code>.
 */
ROX_INTERNAL void response_message_free(HttpResponseMessage *message);

//
// Request
//

typedef struct Request Request;

typedef HttpResponseMessage *(*request_send_get_func)(void *target, Request *request, RequestData *data);

typedef HttpResponseMessage *(*request_send_post_func)(void *target, Request *request, RequestData *data);

typedef HttpResponseMessage *(*request_send_post_json_func)(void *target, Request *request, const char *uri,
                                                            cJSON *json);

/**
 * The following is needed basically for unit testing.
 */
typedef struct RequestConfig {
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
ROX_INTERNAL Request *request_create(RequestConfig *config);

/**
 * @param request Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL HttpResponseMessage *request_send_get(Request *request, RequestData *data);

/**
 * @param request Not <code>NULL</code>.
 * @param uri Not <code>NULL</code>.
 * @param json Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL HttpResponseMessage *request_send_post_json(Request *request, const char *uri, cJSON *json);

/**
 * @param request Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL HttpResponseMessage *request_send_post(Request *request, RequestData *data);

/**
 * @param request Not <code>NULL</code>.
 */
ROX_INTERNAL void request_free(Request *request);

//
// ConfigurationFetcher
//

typedef struct ConfigurationFetcher ConfigurationFetcher;

/**
 * @param request Not <code>NULL</code>.
 * @param sdk_settings Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @param invoker Not <code>NULL</code>.
 * @param reporter Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL ConfigurationFetcher *configuration_fetcher_create(
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
ROX_INTERNAL ConfigurationFetcher *configuration_fetcher_create_roxy(
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
ROX_INTERNAL ConfigurationFetchResult *configuration_fetcher_fetch(ConfigurationFetcher *fetcher);

/**
 * @param fetcher Not <code>NULL</code>.
 */
ROX_INTERNAL void configuration_fetcher_free(ConfigurationFetcher *fetcher);
