#pragma once

#include "core/network.h"
#include "rollout.h"

//
// Requests
//

typedef struct RequestTestFixture {
    int status_to_return_to_get;
    const char *data_to_return_to_get;
    int status_to_return_to_post;
    const char *data_to_return_to_post;
    int status_to_return_to_post_json;
    const char *data_to_return_to_post_json;
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

