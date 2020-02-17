#include <assert.h>
#include <check.h>
#include "fixtures.h"
#include "util.h"

static HttpResponseMessage *_test_request_send_get_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data);
    RequestTestFixture *ctx = (RequestTestFixture *) target;
    ++ctx->times_get_sent;
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

static HttpResponseMessage *_test_request_send_post_func(void *target, Request *request, RequestData *data) {
    assert(target);
    assert(request);
    assert(data);
    RequestTestFixture *ctx = (RequestTestFixture *) target;
    ++ctx->times_post_sent;
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

RequestTestFixture *ROX_INTERNAL request_test_fixture_create() {
    RequestTestFixture *ctx = calloc(1, sizeof(RequestTestFixture));
    RequestConfig request_config = {ctx, _test_request_send_get_func, &_test_request_send_post_func, NULL};
    ctx->request = request_create(&request_config);
    return ctx;
}

void ROX_INTERNAL request_test_fixture_free(RequestTestFixture *ctx) {
    assert(ctx);
    if (ctx->last_get_uri) {
        free(ctx->last_get_uri);
    }
    if (ctx->last_post_uri) {
        free(ctx->last_post_uri);
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

static void _log_record_free(LogRecord *record) {
    assert(record);
    free(record->message);
    free(record);
}

static void _test_logging_handler(void *target, RoxLogMessage *message) {
    assert(target);
    LoggingTestFixture *fixture = (LoggingTestFixture *) target;
    LogRecord *record = calloc(1, sizeof(LogRecord));
    record->level = message->level;
    record->message = mem_copy_str(message->message);
    list_add(fixture->log_records, record);
}

LoggingTestFixture *ROX_INTERNAL logging_test_fixture_create(RoxLogLevel log_level) {
    LoggingTestFixture *fixture = calloc(1, sizeof(LoggingTestFixture));
    list_new(&fixture->log_records);
    RoxLoggingConfig cfg = {log_level, fixture, &_test_logging_handler};
    rox_logging_init(&cfg);
    return fixture;
}

void ROX_INTERNAL logging_test_fixture_free(LoggingTestFixture *fixture) {
    assert(fixture);
    list_destroy_cb(fixture->log_records, (void (*)(void *)) &_log_record_free);
}

void ROX_INTERNAL logging_test_fixture_check_no_messages(LoggingTestFixture *fixture, RoxLogLevel log_level) {
    assert(fixture);
    LIST_FOREACH(item, fixture->log_records, {
        LogRecord *log_record = (LogRecord *) item;
        ck_assert(log_record->level < log_level);
    })
}

void ROX_INTERNAL logging_test_fixture_check_no_errors(LoggingTestFixture *fixture) {
    assert(fixture);
    logging_test_fixture_check_no_messages(fixture, RoxLogLevelError);
}

void ROX_INTERNAL logging_test_fixture_check_log_message(
        LoggingTestFixture *fixture, RoxLogLevel log_level, const char *message) {
    assert(fixture);
    LIST_FOREACH(item, fixture->log_records, {
        LogRecord *log_record = (LogRecord *) item;
        if (log_record->level == log_level) {
            ck_assert(str_starts_with(log_record->message, message));
            return;
        }
    })
    ck_assert(false); // no log record found
}
