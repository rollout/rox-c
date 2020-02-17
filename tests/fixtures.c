#include <assert.h>
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
    if (ctx->last_get_params) {
        rox_map_free_with_values(ctx->last_get_params);
    }
    if (ctx->last_post_params) {
        rox_map_free_with_values(ctx->last_post_params);
    }
    request_free(ctx->request);
    free(ctx);
}
