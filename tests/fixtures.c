#include <assert.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "fixtures.h"
#include "util.h"
#include "collections.h"
#include "server.h"

#ifdef ROX_CLIENT

#include "rox/storage.h"

#endif

// Network

static HttpResponseMessage *test_request_send_get_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data != NULL);
    RequestTestFixture *ctx = (RequestTestFixture *) target;
    ++ctx->times_get_sent;
    if (ctx->last_get_uri) {
        free(ctx->last_get_uri);
    }
    if (ctx->last_get_params) {
        rox_map_free_with_values(ctx->last_get_params);
    }
    ctx->last_get_uri = mem_copy_str(data->url);
    ctx->last_get_params = data->params ? mem_deep_copy_str_value_map(data->params) : NULL;
    if (!ctx->status_to_return_to_get) {
        return NULL;
    }
    return response_message_create(
            ctx->status_to_return_to_get,
            ctx->data_to_return_to_get
            ? mem_copy_str(ctx->data_to_return_to_get)
            : NULL);
}

static HttpResponseMessage *test_request_send_post_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data != NULL);
    RequestTestFixture *ctx = (RequestTestFixture *) target;
    ++ctx->times_post_sent;
    if (ctx->last_post_uri) {
        free(ctx->last_post_uri);
    }
    if (ctx->last_post_params) {
        rox_map_free_with_values(ctx->last_post_params);
    }
    ctx->last_post_uri = mem_copy_str(data->url);
    ctx->last_post_params = data->params ? mem_deep_copy_str_value_map(data->params) : NULL;
    if (!ctx->status_to_return_to_post) {
        return NULL;
    }
    return response_message_create(
            ctx->status_to_return_to_post,
            ctx->data_to_return_to_post
            ? mem_copy_str(ctx->data_to_return_to_post)
            : NULL);
}

static HttpResponseMessage *
test_request_send_post_json_func(void *target, Request *request, const char *uri, cJSON *json) {
    assert(target);
    assert(request);
    assert(uri);
    assert(json);
    RequestTestFixture *ctx = (RequestTestFixture *) target;
    if (!ctx->status_to_return_to_post_json) {
        return NULL;
    }
    if (ctx->last_post_json_uri) {
        free(ctx->last_post_json_uri);
    }
    ctx->last_post_json_uri = mem_copy_str(uri);
    return response_message_create(
            ctx->status_to_return_to_post_json,
            ctx->data_to_return_to_post_json
            ? mem_copy_str(ctx->data_to_return_to_post_json)
            : NULL);
}

ROX_INTERNAL RequestTestFixture *request_test_fixture_create() {
    RequestTestFixture *ctx = calloc(1, sizeof(RequestTestFixture));
    RequestConfig config = {ctx, test_request_send_get_func, &test_request_send_post_func,
                            &test_request_send_post_json_func};
    ctx->config = config;
    ctx->request = request_create(&ctx->config);
    return ctx;
}

ROX_INTERNAL void request_test_fixture_free(RequestTestFixture *ctx) {
    assert(ctx);
    if (ctx->last_get_uri) {
        free(ctx->last_get_uri);
    }
    if (ctx->last_post_uri) {
        free(ctx->last_post_uri);
    }
    if (ctx->last_post_json_uri) {
        free(ctx->last_post_json_uri);
    }
    if (ctx->last_get_params) {
        rox_map_free_with_values(ctx->last_get_params);
    }
    if (ctx->last_post_params) {
        rox_map_free_with_values(ctx->last_post_params);
    }
    request_free(ctx->request);
    free(ctx);
}

//
// Logging
//

static void log_record_free(LogRecord *record) {
    assert(record);
    free(record->message);
    free(record);
}

static void test_logging_handler(void *target, RoxLogMessage *message) {
    assert(target);
    LoggingTestFixture *fixture = (LoggingTestFixture *) target;
    LogRecord *record = calloc(1, sizeof(LogRecord));
    record->level = message->level;
    record->message = mem_copy_str(message->message);
    rox_list_add(fixture->log_records, record);

    FILE *stream = message->level == RoxLogLevelDebug ? stdout : stderr;
    fprintf(stream,
            "%lu %s:%d [%s] %s\n",
            (long) current_time_millis(),
            message->file,
            message->line,
            message->level_name,
            message->message);
    fflush(stream);
}

ROX_INTERNAL LoggingTestFixture *logging_test_fixture_create(RoxLogLevel log_level) {
    LoggingTestFixture *fixture = calloc(1, sizeof(LoggingTestFixture));
    fixture->log_records = rox_list_create();
    RoxLoggingConfig cfg = {log_level, fixture, &test_logging_handler, true};
    rox_logging_init(&cfg);
    return fixture;
}

ROX_INTERNAL void logging_test_fixture_free(LoggingTestFixture *fixture) {
    assert(fixture);
    rox_list_free_cb(fixture->log_records, (void (*)(void *)) &log_record_free);
    free(fixture);
}

ROX_INTERNAL void logging_test_fixture_check_no_messages(LoggingTestFixture *fixture, RoxLogLevel log_level) {
    assert(fixture);
    ROX_LIST_FOREACH(item, fixture->log_records, {
        LogRecord *log_record = (LogRecord *) item;
        ck_assert(log_record->level < log_level);
    })
}

ROX_INTERNAL void logging_test_fixture_check_no_errors(LoggingTestFixture *fixture) {
    assert(fixture);
    logging_test_fixture_check_no_messages(fixture, RoxLogLevelError);
}

ROX_INTERNAL void logging_test_fixture_check_log_message(
        LoggingTestFixture *fixture, RoxLogLevel log_level, const char *message) {
    assert(fixture);
    ROX_LIST_FOREACH(item, fixture->log_records, {
        LogRecord *log_record = (LogRecord *) item;
        if (log_record->level == log_level) {
            ck_assert(str_starts_with(log_record->message, message));
            ROX_LIST_FOREACH_RETURN;
        }
    })
    ck_assert(false); // no log record found
}

// Flag integration tests

static void test_impression_handler(
        void *target,
        RoxReportingValue *value,
        RoxContext *context) {
    FlagTestFixture *ctx = (FlagTestFixture *) target;
    ctx->test_impression_raised = true;
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
    }
    if (ctx->imp_context_value) {
        rox_dynamic_value_free(ctx->imp_context_value);
        ctx->imp_context_value = NULL;
    }
    ctx->last_impression_value = mem_copy_str(value->value);
    ctx->last_impression_targeting = value->targeting;
    if (ctx->imp_context_key) {
        ctx->imp_context_value = rox_context_get(context, ctx->imp_context_key);
    }
}

static void test_flag_action(void *target) {
    FlagTestFixture *ctx = (FlagTestFixture *) target;
    ctx->test_flag_action_called = true;
}

static FlagTestFixture *flag_test_fixture_create_new() {
    FlagTestFixture *fixture = calloc(1, sizeof(FlagTestFixture));
    fixture->request = request_test_fixture_create();
    fixture->logging = logging_test_fixture_create(RoxLogLevelDebug);
    fixture->storage_values = ROX_EMPTY_MAP;
    rox_set_default_request_config(&fixture->request->config);
    return fixture;
}

static void flag_test_fixture_setup_with_options(FlagTestFixture *fixture, RoxOptions *options) {
    rox_options_set_impression_handler(options, fixture, &test_impression_handler);
    rox_options_set_roxy_url(options, "http://localhost");
    RoxStateCode status = rox_setup("any", options);
    ck_assert_int_eq(RoxInitialized, status);
}

#ifdef ROX_CLIENT

static void in_memory_storage_write(void *target, RoxStorageEntry *entry, const char *data) {
    rox_storage_entry_set_meta_data(entry, mem_copy_str(data), free);
}

static char *in_memory_storage_read(void *target, RoxStorageEntry *entry) {
    void *data = rox_storage_entry_get_meta_data(entry);
    if (!data) {
        FlagTestFixture *fixture = target;
        const char *name = rox_storage_entry_get_name(entry);
        rox_map_get(fixture->storage_values, (void *) name, &data);
        if (data) {
            return mem_copy_str(data);
        }
    }
    return data;
}

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_options_and_storage(
        RoxOptions *options,
        const char *data) {
    FlagTestFixture *fixture = flag_test_fixture_create_new();
    if (data) {
        rox_map_add(fixture->storage_values, "overrides", data);
    }
    RoxStorageConfig storage_config = {
            NULL,
            fixture,
            NULL,
            NULL,
            in_memory_storage_write,
            in_memory_storage_read,
            NULL
    };
    rox_options_set_storage_config(options, &storage_config);
    flag_test_fixture_setup_with_options(fixture, options);
    return fixture;
}

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_storage(const char *data) {
    return flag_test_fixture_create_with_options_and_storage(rox_options_create(), data);
}

#endif

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_options(RoxOptions *options) {
    FlagTestFixture *fixture = flag_test_fixture_create_new();
    flag_test_fixture_setup_with_options(fixture, options);
    return fixture;
}

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create() {
    return flag_test_fixture_create_with_options(rox_options_create());
}

ROX_INTERNAL void flag_test_fixture_handle_enabled_callback(
        FlagTestFixture *ctx,
        RoxStringBase *flag) {
    rox_enabled_do(flag, ctx, &test_flag_action);
}

ROX_INTERNAL void flag_test_fixture_handle_disable_callback(
        FlagTestFixture *ctx,
        RoxStringBase *flag) {
    rox_disabled_do(flag, ctx, &test_flag_action);
}

ROX_INTERNAL void flag_test_fixture_set_experiments(FlagTestFixture *ctx, RoxMap *conditions) {
    assert(ctx);
    assert(conditions);

    cJSON *experiment_arr = ROX_EMPTY_JSON_ARRAY;

    ROX_MAP_FOREACH(key, value, conditions, {
        cJSON_AddItemToArray(experiment_arr, ROX_JSON_OBJECT(
                "_id", ROX_JSON_STRING(key),
                "name", ROX_JSON_STRING(key),
                "archived", ROX_JSON_FALSE,
                "featureFlags", ROX_JSON_ARRAY(ROX_JSON_OBJECT("name", ROX_JSON_STRING(key))),
                "deploymentConfiguration", ROX_JSON_OBJECT("condition", ROX_JSON_STRING(value)),
                "labels", ROX_EMPTY_JSON_ARRAY,
                "stickinessProperty", ROX_JSON_STRING("rox.distinct_id")
        ));
    })

    cJSON *data_json = ROX_JSON_OBJECT(
            "api_version", ROX_JSON_STRING("1.9.0"),
            "creation_date", ROX_JSON_STRING("2021-02-25T05:22:35.363Z"),
            "platform", ROX_JSON_STRING("C"),
            "application", ROX_JSON_STRING("test"),
            "remoteVariables", ROX_EMPTY_JSON_ARRAY,
            "targetGroups", ROX_EMPTY_JSON_ARRAY,
            "experiments", experiment_arr);

    char *data_json_str = ROX_JSON_SERIALIZE(data_json);

    cJSON *json = ROX_JSON_OBJECT(
            "data", ROX_JSON_STRING(data_json_str),
            "signature_v0", ROX_JSON_STRING("12345"),
            "signed_date", ROX_JSON_STRING("2021-02-25T05:22:35.370Z"));

    if (ctx->request->data_to_return_to_get) {
        free(ctx->request->data_to_return_to_get);
        ctx->request->data_to_return_to_get = NULL;
    }

    ctx->request->status_to_return_to_get = 200;
    ctx->request->data_to_return_to_get = ROX_JSON_SERIALIZE(json);

    cJSON_Delete(data_json);
    cJSON_Delete(json);
    rox_map_free(conditions);
    free(data_json_str);

    rox_fetch();
}

ROX_INTERNAL void flag_test_fixture_set_flag_experiment(
        FlagTestFixture *ctx,
        RoxStringBase *flag,
        const char *condition) {

    flag_test_fixture_set_experiments(ctx, ROX_MAP(
            variant_get_name(flag),
            condition));
}

ROX_INTERNAL void flag_test_fixture_check_no_impression(FlagTestFixture *ctx) {
    ck_assert(!ctx->test_impression_raised);
    ck_assert_ptr_null(ctx->last_impression_value);
}

ROX_INTERNAL void flag_test_fixture_check_impression(FlagTestFixture *ctx, const char *value) {
    ck_assert(ctx->test_impression_raised);
    ck_assert_str_eq(value, ctx->last_impression_value);
    ctx->test_impression_raised = false;
    free(ctx->last_impression_value);
    ctx->last_impression_value = NULL;
}

ROX_INTERNAL void check_impression_ex(FlagTestFixture *ctx, const char *value, bool targeting) {
    flag_test_fixture_check_impression(ctx, value);
    if (targeting) {
        ck_assert(ctx->last_impression_targeting);
    } else {
        ck_assert(!ctx->last_impression_targeting);
    }
}

ROX_INTERNAL void flag_test_fixture_free(FlagTestFixture *ctx) {
    if (ctx->request->data_to_return_to_get) {
        free(ctx->request->data_to_return_to_get);
    }
    rox_map_free(ctx->storage_values);
    rox_set_default_request_config(NULL);
    request_test_fixture_free(ctx->request);
    logging_test_fixture_free(ctx->logging);
    if (ctx->last_impression_value) {
        free(ctx->last_impression_value);
        ctx->last_impression_value = NULL;
    }
    if (ctx->imp_context_value) {
        rox_dynamic_value_free(ctx->imp_context_value);
        ctx->imp_context_value = NULL;
    }
    free(ctx);
    rox_shutdown();
}
