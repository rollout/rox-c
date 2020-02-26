#pragma once

#include <collectc/hashtable.h>
#include "core/network.h"
#include "rollout.h"

//
// Requests
//

typedef struct ROX_INTERNAL RequestTestFixture {
    int status_to_return_to_get;
    const char *data_to_return_to_get;
    int status_to_return_to_post;
    const char *data_to_return_to_post;
    int status_to_return_to_post_json;
    const char *data_to_return_to_post_json;
    int times_get_sent;
    char *last_get_uri;
    HashTable *last_get_params;
    int times_post_sent;
    char *last_post_uri;
    int times_post_json_sent;
    char *last_post_json_uri;
    HashTable *last_post_params;
    RequestConfig config;
    Request *request;
} RequestTestFixture;

RequestTestFixture *ROX_INTERNAL request_test_fixture_create();

void ROX_INTERNAL request_test_fixture_free(RequestTestFixture *fixture);

//
// Logging
//

typedef struct ROX_INTERNAL LogRecord {
    RoxLogLevel level;
    char *message;
} LogRecord;

typedef struct ROX_INTERNAL LoggingTestFixture {
    List *log_records;
} LoggingTestFixture;

LoggingTestFixture *ROX_INTERNAL logging_test_fixture_create(RoxLogLevel log_level);

void ROX_INTERNAL logging_test_fixture_check_no_errors(LoggingTestFixture *fixture);

void ROX_INTERNAL logging_test_fixture_check_no_messages(LoggingTestFixture *fixture, RoxLogLevel log_level);

void ROX_INTERNAL logging_test_fixture_check_log_message(
        LoggingTestFixture *fixture, RoxLogLevel log_level, const char *message);

void ROX_INTERNAL logging_test_fixture_free(LoggingTestFixture *fixture);

