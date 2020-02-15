#include <cjson/cJSON.h>
#include <assert.h>
#include <stdio.h>
#include "core/consts.h"
#include "core/logging.h"
#include "reporting.h"
#include "core/client.h"
#include "util.h"

struct ROX_INTERNAL XErrorReporter {
    Request *request;
    DeviceProperties *properties;
    BUID *buid;
};

/**
 * @param request Not <code>NULL</code>.
 * @param properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
XErrorReporter *ROX_INTERNAL x_error_reporter_create(
        Request *request,
        DeviceProperties *properties,
        BUID *buid) {

    assert(request);
    assert(properties);
    assert(buid);

    XErrorReporter *reporter = calloc(1, sizeof(XErrorReporter));
    reporter->request = request;
    reporter->properties = properties;
    reporter->buid = buid;
    return reporter;
}

void ROX_INTERNAL x_error_reporter_free(XErrorReporter *reporter) {
    assert(reporter);
    free(reporter);
}

#define BUGSNAG_NOTIFY_URL "https://notify.bugsnag.com"

static void _x_error_reporter_send_error(XErrorReporter *x_reporter, cJSON *json) {
    assert(x_reporter);
    assert(json);

    ROX_DEBUG("Sending bugsnag error report...");
    request_send_post_json(x_reporter->request, BUGSNAG_NOTIFY_URL, json);
    ROX_DEBUG("Bugsnag error report was sent");
}

#undef BUGSNAG_NOTIFY_URL

static void x_error_reporter_add_meta_data(XErrorReporter *x_reporter, const char *message, cJSON *ev) {
    cJSON_AddItemToObject(ev, "metaData", ROX_JSON_OBJECT("data", ROX_JSON_OBJECT(
            "message", ROX_JSON_STRING(message),
            "deviceId", ROX_JSON_STRING(device_properties_get_distinct_id(x_reporter->properties)),
            "buid", ROX_JSON_STRING(buid_get_value(x_reporter->buid))
    )));
}

static void x_error_reporter_add_api_key(cJSON *payload) {
    cJSON_AddStringToObject(payload, "apiKey", "9569ec14f61546c6aa2a97856492bf4d");
}

static cJSON *x_error_reporter_add_payload_version() {
    return ROX_JSON_OBJECT("payloadVersion", ROX_JSON_STRING("2"));
}

static void x_error_reporter_add_user(const char *id, const char *rolloutKey, cJSON *ev) {
    cJSON_AddItemToObject(ev, "user", ROX_JSON_OBJECT(id, ROX_JSON_STRING(rolloutKey)));
}

static void x_error_reporter_add_exceptions(const char *message, const char *file, int line, cJSON *ev) {
    cJSON_AddItemToObject(ev, "exceptions", ROX_JSON_ARRAY(
            ROX_JSON_OBJECT(
                    "errorClass", ROX_JSON_STRING("undefined"),
                    "message", ROX_JSON_STRING(message),
                    "stacktrace", ROX_JSON_ARRAY(
                            ROX_JSON_OBJECT(
                                    "file", ROX_JSON_STRING(file),
                                    "method", ROX_JSON_STRING("unknown"),
                                    "lineNumber", ROX_JSON_INT(line),
                                    "columnNumber", ROX_JSON_INT(-1))
                    ))
    ));
}

static void x_error_reporter_add_app(XErrorReporter *x_reporter, cJSON *ev) {
    cJSON_AddItemToObject(ev, "app", ROX_JSON_OBJECT(
            "releaseStage", ROX_JSON_STRING(device_properties_get_rollout_environment(x_reporter->properties)),
            "version", ROX_JSON_STRING(device_properties_get_lib_version(x_reporter->properties))
    ));
}

static void x_error_reporter_add_event(
        XErrorReporter *x_reporter,
        const char *message,
        const char *file,
        int line,
        cJSON *events) {

    cJSON *ev = x_error_reporter_add_payload_version();
    x_error_reporter_add_exceptions(message, file, line, ev);
    x_error_reporter_add_user("id", device_properties_get_rollout_key(x_reporter->properties), ev);
    x_error_reporter_add_meta_data(x_reporter, message, ev);
    x_error_reporter_add_app(x_reporter, ev);
    cJSON_AddItemToArray(events, ev);
}

static void x_error_reporter_add_events(
        XErrorReporter *x_reporter,
        const char *message,
        const char *file,
        int line,
        cJSON *payload) {

    cJSON *evs = cJSON_CreateArray();
    x_error_reporter_add_event(x_reporter, message, file, line, evs);
    cJSON_AddItemToObject(payload, "events", evs);
}

static void x_error_reporter_add_notifier(XErrorReporter *x_reporter, cJSON *payload) {
    cJSON_AddItemToObject(payload, "notifier", ROX_JSON_OBJECT(
            "name", ROX_JSON_STRING("Rollout C SDK"),
            "version", ROX_JSON_STRING(device_properties_get_lib_version(x_reporter->properties))
    ));
}

static cJSON *x_error_reporter_create_payload(
        XErrorReporter *x_reporter,
        const char *message,
        const char *file,
        int line) {

    assert(message);
    assert(file);
    assert(line);

    cJSON *json = cJSON_CreateObject();
    x_error_reporter_add_api_key(json);
    x_error_reporter_add_notifier(x_reporter, json);
    x_error_reporter_add_events(x_reporter, message, file, line, json);
    return json;
}

#define ROX_X_ERROR_REPORTER_MESSAGE_BUFFER_SIZE 1024

void ROX_INTERNAL x_error_reporter_report(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list args) {

    assert(target);
    assert(reporter);
    assert(file);
    assert(line);
    assert(fmt);

    XErrorReporter *x_reporter = (XErrorReporter *) target;
    if (str_equals(device_properties_get_rollout_environment(x_reporter->properties), ROX_ENV_MODE_LOCAL)) {
        return;
    }

    char message[ROX_X_ERROR_REPORTER_MESSAGE_BUFFER_SIZE];
    vsprintf_s(message, ROX_X_ERROR_REPORTER_MESSAGE_BUFFER_SIZE, fmt, args);

    ROX_ERROR("Error report: %s", message);
    cJSON *json = x_error_reporter_create_payload(x_reporter, message, file, line);
    _x_error_reporter_send_error(x_reporter, json);
    cJSON_Delete(json);
}

#undef ROX_X_ERROR_REPORTER_MESSAGE_BUFFER_SIZE
