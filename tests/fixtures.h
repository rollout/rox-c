#pragma once

#include "core/network.h"
#include "rox/server.h"

//
// Requests
//

typedef struct RequestTestFixture {
    int status_to_return_to_get;
    char *data_to_return_to_get;
    int status_to_return_to_post;
    char *data_to_return_to_post;
    int status_to_return_to_post_json;
    char *data_to_return_to_post_json;
    int times_get_sent;
    char *last_get_uri;
    RoxMap *last_get_params;
    int times_post_sent;
    char *last_post_uri;
    int times_post_json_sent;
    char *last_post_json_uri;
    RoxMap *last_post_params;
    RequestConfig config;
    Request *request;
} RequestTestFixture;

ROX_INTERNAL RequestTestFixture *request_test_fixture_create();

ROX_INTERNAL void request_test_fixture_free(RequestTestFixture *fixture);

//
// Logging
//

typedef struct LogRecord {
    RoxLogLevel level;
    char *message;
} LogRecord;

typedef struct LoggingTestFixture {
    RoxList *log_records;
} LoggingTestFixture;

ROX_INTERNAL LoggingTestFixture *logging_test_fixture_create(RoxLogLevel log_level);

ROX_INTERNAL void logging_test_fixture_check_no_errors(LoggingTestFixture *fixture);

ROX_INTERNAL void logging_test_fixture_check_no_messages(LoggingTestFixture *fixture, RoxLogLevel log_level);

ROX_INTERNAL void logging_test_fixture_check_log_message(
        LoggingTestFixture *fixture, RoxLogLevel log_level, const char *message);

ROX_INTERNAL void logging_test_fixture_free(LoggingTestFixture *fixture);

// Flag integration tests

typedef struct FlagTestFixture {
    bool test_impression_raised;
    char *last_impression_value;
    bool last_impression_targeting;
    const char *imp_context_key;
    RoxDynamicValue *imp_context_value;
    bool test_flag_action_called;
    RequestTestFixture *request;
    LoggingTestFixture *logging;
    RoxMap *storage_values;
} FlagTestFixture;

#ifdef ROX_CLIENT
ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_options_and_storage(
        RoxOptions *options,
        const char* data);

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_storage(const char *data);
#endif

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create_with_options(RoxOptions *options);

ROX_INTERNAL FlagTestFixture *flag_test_fixture_create();

ROX_INTERNAL void flag_test_fixture_handle_enabled_callback(
        FlagTestFixture *ctx,
        RoxStringBase *variant);

ROX_INTERNAL void flag_test_fixture_handle_disable_callback(
        FlagTestFixture *ctx,
        RoxStringBase *variant);

ROX_INTERNAL void flag_test_fixture_set_experiments(FlagTestFixture *ctx, RoxMap *conditions);

ROX_INTERNAL void flag_test_fixture_set_flag_experiment(
        FlagTestFixture *ctx,
        RoxStringBase *flag,
        const char *condition);

ROX_INTERNAL void flag_test_fixture_check_no_impression(FlagTestFixture *ctx);

ROX_INTERNAL void flag_test_fixture_check_impression(FlagTestFixture *ctx, const char *value);

ROX_INTERNAL void check_impression_ex(FlagTestFixture *ctx, const char *value, bool targeting);

ROX_INTERNAL void flag_test_fixture_free(FlagTestFixture *ctx);
