#pragma once

#include <collectc/hashtable.h>
#include "core/network.h"
#include "roxapi.h"

typedef struct ROX_INTERNAL RequestTestFixture {
    int status_to_return_to_get;
    const char *data_to_return_to_get;
    int status_to_return_to_post;
    const char *data_to_return_to_post;
    int times_get_sent;
    char *last_get_uri;
    HashTable *last_get_params;
    int times_post_sent;
    char *last_post_uri;
    HashTable *last_post_params;
    Request *request;
} RequestTestFixture;

RequestTestFixture *ROX_INTERNAL request_test_fixture_create();

void ROX_INTERNAL request_test_fixture_free(RequestTestFixture *fixture);