#include <check.h>
#include <assert.h>
#include "roxtests.h"
#include "xpack/notifications.h"

START_TEST (test_sse) {

    NotificationListenerConfig config = {
            "https://qax-push.rollout.io/sse",
            "5b3356d00d81206da3055bc0"
    };

    config.current_thread = true; // run in a single threaded mode
    NotificationListener *listener = notification_listener_create(&config);
    notification_listener_start(listener);
    notification_listener_free(listener);
}

END_TEST

typedef struct ROX_INTERNAL ListenerEventTestContext {
    NotificationListenerConfig *config;
    NotificationListener *listener;
    List *events;
} ListenerEventTestContext;

static void _test_notification_listener_event_handler(void *target, NotificationListenerEvent *event) {
    assert(target);
    assert(event);
    ListenerEventTestContext *ctx = (ListenerEventTestContext *) target;
    list_add(ctx->events, notification_listener_event_copy(event));
}

static void _check_notification_listener_event(
        ListenerEventTestContext *ctx,
        const char *name,
        const char *data) {

    assert(ctx);

    ck_assert_int_eq(1, list_size(ctx->events));

    NotificationListenerEvent *event;
    ck_assert_int_eq(list_get_first(ctx->events, (void **) &event), CC_OK);
    ck_assert_str_eq(name, event->event_name);

    if (data) {
        ck_assert_str_eq(data, event->data);
    } else {
        ck_assert_ptr_null(event->data);
    }
}

static ListenerEventTestContext *listener_event_test_context_create() {
    ListenerEventTestContext *ctx = calloc(1, sizeof(ListenerEventTestContext));
    ctx->config = calloc(1, sizeof(NotificationListenerConfig));
    ctx->config->listen_url = "test";
    ctx->config->app_key = "test";
    ctx->config->testing = true;
    ctx->listener = notification_listener_create(ctx->config);
    list_new(&ctx->events);
    notification_listener_on(ctx->listener, "test_event", ctx, &_test_notification_listener_event_handler);
    return ctx;
}

static void listener_event_test_context_free(ListenerEventTestContext *ctx) {
    assert(ctx);
    free(ctx->config);
    notification_listener_free(ctx->listener);
    list_destroy_cb(ctx->events, (void (*)(void *)) &notification_listener_event_free);
    free(ctx);
}

START_TEST (test_listener_events_empty_line) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "\n\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_empty_line_cr) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "\n\r\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_empty_line_cr2) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "\n\r\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_comment) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, ":ok\n\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_comment_cr2) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, ":ok\n\r\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_comment_cr) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, ":ok\r\n\r\n");
    ck_assert_int_eq(list_size(ctx->events), 0);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_no_data) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\n\n");
    _check_notification_listener_event(ctx, "test_event", NULL);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_no_data_cr) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\n\r\n");
    _check_notification_listener_event(ctx, "test_event", NULL);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_no_data_cr2) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", NULL);
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_single_line_data) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\n\n");
    _check_notification_listener_event(ctx, "test_event", "test");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_single_line_data_cr) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_single_line_data_cr2) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_single_line_data_cr3) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\r\ndata: test\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_multi_line_data) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\ndata: me\n\n");
    _check_notification_listener_event(ctx, "test_event", "test\nme");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_multi_line_data_cr) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\ndata: me\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test\nme");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_multi_line_data_cr2) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\ndata: test\ndata: me\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test\nme");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_multi_line_data_cr3) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event: test_event\r\ndata: test\r\ndata: me\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test\nme");
    listener_event_test_context_free(ctx);
}

END_TEST

START_TEST (test_listener_events_multi_line_data_no_space_after_field_name) {
    ListenerEventTestContext *ctx = listener_event_test_context_create();
    notification_listener_test(ctx->listener, "event:test_event\r\ndata:test\r\ndata: me\r\n\r\n");
    _check_notification_listener_event(ctx, "test_event", "test\nme");
    listener_event_test_context_free(ctx);
}

END_TEST

ROX_TEST_SUITE(
//        ROX_TEST_CASE(test_sse), // To be invoked manually, just for checking if everything works
        ROX_TEST_CASE(test_listener_events_empty_line),
        ROX_TEST_CASE(test_listener_events_empty_line_cr),
        ROX_TEST_CASE(test_listener_events_empty_line_cr2),
        ROX_TEST_CASE(test_listener_events_comment),
        ROX_TEST_CASE(test_listener_events_comment_cr),
        ROX_TEST_CASE(test_listener_events_comment_cr2),
        ROX_TEST_CASE(test_listener_events_no_data),
        ROX_TEST_CASE(test_listener_events_no_data_cr),
        ROX_TEST_CASE(test_listener_events_no_data_cr2),
        ROX_TEST_CASE(test_listener_events_single_line_data),
        ROX_TEST_CASE(test_listener_events_single_line_data_cr),
        ROX_TEST_CASE(test_listener_events_single_line_data_cr2),
        ROX_TEST_CASE(test_listener_events_single_line_data_cr3),
        ROX_TEST_CASE(test_listener_events_multi_line_data),
        ROX_TEST_CASE(test_listener_events_multi_line_data_cr),
        ROX_TEST_CASE(test_listener_events_multi_line_data_cr2),
        ROX_TEST_CASE(test_listener_events_multi_line_data_cr3),
        ROX_TEST_CASE(test_listener_events_multi_line_data_no_space_after_field_name)
)
