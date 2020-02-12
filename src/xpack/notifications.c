#include <collectc/hashtable.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <ctype.h>
#include "notifications.h"
#include "util.h"
#include "os.h"

#ifdef ROX_WINDOWS

#include <windows.h>

#else
#include <unistd.h>
#endif

//
// DisconnectEventArgs
//

typedef struct ROX_INTERNAL DisconnectEventArgs {
    int reconnect_delay;
} DisconnectEventArgs;

//
// EventSourceMessageEventArgs
//

typedef struct ROX_INTERNAL EventSourceMessageEventArgs {
    const char *id;
    const char *event;
    const char *message;
} EventSourceMessageEventArgs;

//
// EventSourceReader
//

typedef void (*event_source_reader_on_message_func)(void *target, EventSourceMessageEventArgs *args);

typedef struct ROX_INTERNAL EventSourceReader {
    char *url;
    void *target;
    event_source_reader_on_message_func on_message;
    bool reading;
    int reconnect_timeout_millis;
    pthread_t thread;
    CURL *curl;
    char *last_event_id;
    bool skip_ssl_cert_verification;
} EventSourceReader;

void ROX_INTERNAL _event_source_reader_stop(EventSourceReader *reader) {
    assert(reader);
    if (reader->reading) {
        reader->reading = false;
        if (reader->curl) {
            curl_easy_cleanup(reader->curl);
            reader->curl = NULL;
        }
    }
}

size_t _event_source_reader_header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t real_size = size * nitems;
    EventSourceReader *reader = (EventSourceReader *) userdata;
    if (str_starts_with(buffer, "Content-Type: ")) {
        if (!str_starts_with(buffer + strlen("Content-Type: "), "text/event-stream")) {
            fprintf(stderr, "Specified URI does not return server-sent events: %s\n", reader->url); // TODO: log
            _event_source_reader_stop(reader);
        }
    }
    return real_size;
}

static void _event_source_reader_fire_event(
        EventSourceReader *reader,
        const char *event,
        const char *message,
        const char *id) {

    assert(reader);
    assert(event);

    fprintf(stdout, "New event: %s [%s] (%s)\n", event, id, message); // TODO: log (debug)
    EventSourceMessageEventArgs *args = calloc(1, sizeof(EventSourceMessageEventArgs));
    args->event = event;
    args->message = message;
    args->id = id;
    reader->on_message(reader->target, args);
    free(args);
}

typedef enum ROX_INTERNAL EventSourceReaderState {
    InitialState = 0,
    ReadingFieldName,
    ReadingSpaceAfterFieldName,
    ReadingFieldValue,
    ReadError
} EventSourceReaderState;

typedef struct ROX_INTERNAL EventSourceReaderFsm {
    EventSourceReaderState state;
    char *event;
    char *message;
    char *id;
    int field_name_start;
    int field_name_end;
    int field_value_start;
    int field_value_end;
} EventSourceReaderFsm;

static void _event_source_reader_cleanup_state(EventSourceReaderFsm *fsm) {
    assert(fsm);
    if (fsm->event) {
        free(fsm->event);
        fsm->event = NULL;
    }
    if (fsm->message) {
        free(fsm->message);
        fsm->message = NULL;
    }
    if (fsm->id) {
        free(fsm->id);
        fsm->id = NULL;
    }
}

static void _event_source_reader_reset_state(EventSourceReaderFsm *fsm, int index) {
    assert(fsm);
    _event_source_reader_cleanup_state(fsm);
    fsm->field_name_start = index;
    fsm->field_name_end = index;
    fsm->field_value_start = index;
    fsm->field_value_end = index;
    fsm->state = InitialState;
}

#define DEFAULT_RECONNECT_TIMEOUT_SECONDS 3

static void _event_source_reader_message_read(
        EventSourceReader *reader,
        EventSourceReaderFsm *fsm,
        const char *src,
        size_t bytes_read) {

    assert(reader);
    assert(fsm);
    assert(src);

    int field_value_len = fsm->field_value_end - fsm->field_value_start;

    if (str_eq_n(src, fsm->field_name_start, fsm->field_name_end, "event")) {

        assert(!fsm->event);
        fsm->event = mem_str_substring_n(src, bytes_read, fsm->field_value_start, field_value_len);

    } else if (str_eq_n(src, fsm->field_name_start, fsm->field_name_end, "data")) {

        char *data = mem_str_substring_n(src, bytes_read, fsm->field_value_start, field_value_len);
        if (!fsm->message) {
            fsm->message = data;
        } else {
            fsm->message = mem_str_format("%s\n%s", fsm->message, data);
            free(data);
        }

    } else if (str_eq_n(src, fsm->field_name_start, fsm->field_name_end, "id")) {

        assert(!fsm->id);
        fsm->id = mem_str_substring_n(src, bytes_read, fsm->field_value_start, field_value_len);

    } else {

        if (str_eq_n(src, fsm->field_name_start, fsm->field_name_end, "retry")) {

            char *value = mem_str_substring_n(src, bytes_read, fsm->field_value_start, field_value_len);
            int *reconnect_millis = mem_str_to_int(value);
            if (reconnect_millis && *reconnect_millis) {
                reader->reconnect_timeout_millis = *reconnect_millis;
                free(reconnect_millis);
            } else {
                fprintf(stderr, "failed to parse retry field value '%s'\n", value); // TODO: log (warning)
                reader->reconnect_timeout_millis = DEFAULT_RECONNECT_TIMEOUT_SECONDS * 1000;
            }
            free(value);

        } else {

            char *name = mem_str_substring_n(src, bytes_read, fsm->field_name_start,
                                             fsm->field_name_end - fsm->field_name_start);
            fprintf(stderr, "Unknown field name: '%s'\n", name); // TODO: log (warning)
            free(name);
        }
    }
}

static void _event_source_reader_update_state(EventSourceReader *reader, const char *ptr, size_t bytes_read) {
    assert(reader);
    assert(ptr);
    assert(bytes_read >= 0);

    if (bytes_read == 0 || ptr[0] == ':') {
        // ignore comments
        return;
    }

    EventSourceReaderFsm *fsm = calloc(1, sizeof(EventSourceReaderFsm));
    _event_source_reader_reset_state(fsm, 0);

    for (int i = 0; i < bytes_read; ++i) {
        char c = ptr[i];
        switch (fsm->state) {

            case InitialState:
                if (c == '\n') {
                    // double newline, dispatch message and reset for next
                    if (fsm->event) {
                        _event_source_reader_fire_event(reader, fsm->event, fsm->message, fsm->id);
                    }
                    _event_source_reader_reset_state(fsm, ++i);
                } else if (c != '\r') {
                    fsm->field_name_start = i--;
                    fsm->state = ReadingFieldName;
                }
                break;

            case ReadingFieldName:
                if (c == ':') {
                    fsm->field_name_end = i;
                    fsm->state = ReadingSpaceAfterFieldName;
                }
                break;

            case ReadingSpaceAfterFieldName:
                if (c != ' ') {
                    fsm->field_value_start = i--;
                    fsm->state = ReadingFieldValue;
                }
                break;

            case ReadingFieldValue:
                if (c == '\r') {
                    fsm->field_value_end = i;
                } else if (c == '\n') {
                    if (fsm->field_value_end < fsm->field_value_start) { // check if not previously set
                        fsm->field_value_end = i;
                    }
                    _event_source_reader_message_read(reader, fsm, ptr, bytes_read);
                    fsm->state = InitialState;
                }
                break;

            default:
            case ReadError:
                char *line = mem_str_substring_n(ptr, bytes_read, 0, bytes_read);
                fprintf(stderr, "failed to parse line '%s': error at pos %d\n", line, i); // TODO: log (warning)
                free(line);
                i = bytes_read;
                break;
        }
    }

    _event_source_reader_cleanup_state(fsm);
    free(fsm);
}

static size_t _event_source_reader_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t real_size = size * nmemb;
    EventSourceReader *reader = (EventSourceReader *) userdata;
    _event_source_reader_update_state(reader, ptr, nmemb);
    return real_size;
}

static void *_event_source_reader_thread_func(void *ptr) {
    EventSourceReader *reader = (EventSourceReader *) ptr;

    if (reader->curl) {
        curl_easy_cleanup(reader->curl);
    }

    reader->curl = curl_easy_init();

    curl_easy_setopt(reader->curl, CURLOPT_URL, reader->url);
    curl_easy_setopt(reader->curl, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(reader->curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(reader->curl, CURLOPT_ACCEPT_ENCODING, ""); // enable all supported built-in compressions
    curl_easy_setopt(reader->curl, CURLOPT_TIMEOUT, 0);
    curl_easy_setopt(reader->curl, CURLOPT_HTTPGET, true);
    curl_easy_setopt(reader->curl, CURLOPT_WRITEFUNCTION, &_event_source_reader_write_callback);
    curl_easy_setopt(reader->curl, CURLOPT_WRITEDATA, reader);
    curl_easy_setopt(reader->curl, CURLOPT_HEADERFUNCTION, &_event_source_reader_header_callback);
    curl_easy_setopt(reader->curl, CURLOPT_HEADERDATA, reader);
    curl_easy_setopt(reader->curl, CURLOPT_SSL_VERIFYPEER, !reader->skip_ssl_cert_verification);

    struct curl_slist *headers = NULL;

    if (reader->last_event_id) {
        char *last_event_id_header = mem_str_format("Last-Event-Id: %s", reader->last_event_id);
        headers = curl_slist_append(NULL, last_event_id_header);
        curl_easy_setopt(reader->curl, CURLOPT_HTTPHEADER, headers);
        free(last_event_id_header);
    }

    while (reader->reading) {
        fprintf(stdout, "connecting to %s\n", reader->url); // TODO: log
        CURLcode res = curl_easy_perform(reader->curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res)); // TODO: log
        }
        fprintf(stderr, "reconnecting after %d seconds\n", reader->reconnect_timeout_millis); // TODO: log
        int sleep_millis = reader->reconnect_timeout_millis * 1000;
#ifdef ROX_WINDOWS
        Sleep(sleep_millis);
#else
        usleep(sleep_millis * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
    }

    if (headers) {
        curl_slist_free_all(headers);
    }
}

static void _event_source_reader_start(EventSourceReader *reader, bool join) {
    assert(reader);
    if (!reader->reading) {
        reader->reading = (pthread_create(&reader->thread, NULL,
                                          _event_source_reader_thread_func, (void *) reader) == 0);
        if (join) {
            pthread_join(reader->thread, NULL);
        }
    }
}

static EventSourceReader *_event_source_reader_create(
        const char *url,
        void *target,
        event_source_reader_on_message_func on_message) {

    assert(url);
    assert(on_message);

    EventSourceReader *reader = calloc(1, sizeof(EventSourceReader));
    reader->url = mem_copy_str(url);
    reader->target = target;
    reader->on_message = on_message;
    reader->reconnect_timeout_millis = DEFAULT_RECONNECT_TIMEOUT_SECONDS * 1000;
    return reader;
}

#undef DEFAULT_RECONNECT_TIMEOUT_SECONDS

void ROX_INTERNAL _event_source_reader_free(EventSourceReader *reader) {
    assert(reader);
    _event_source_reader_stop(reader);
    free(reader->url);
    free(reader);
}

//
// Event
//

NotificationListenerEvent *ROX_INTERNAL notification_listener_event_create(const char *event_name, const char *data) {
    assert(event_name);
    NotificationListenerEvent *event = calloc(1, sizeof(NotificationListenerEvent));
    event->event_name = event_name;
    event->data = data;
    return event;
}

NotificationListenerEvent *ROX_INTERNAL notification_listener_event_copy(NotificationListenerEvent *event) {
    assert(event);
    NotificationListenerEvent *copy = calloc(1, sizeof(NotificationListenerEvent));
    copy->event_name = mem_copy_str(event->event_name);
    copy->data = event->data ? mem_copy_str(event->data) : NULL;
    return copy;
}

void ROX_INTERNAL notification_listener_event_free(NotificationListenerEvent *event) {
    assert(event);
    free(event);
}

//
// NotificationListener
//

typedef struct NotificationListenerEventHandler {
    void *target;
    notification_listener_event_handler handler;
} NotificationListenerEventHandler;

struct ROX_INTERNAL NotificationListener {
    char *listen_url;
    char *app_key;
    HashTable *handlers; // char* => List* of NotificationListenerEventHandler
    EventSourceReader *reader;
    // debugging options
    bool testing;
    bool current_thread;
};

static void _notification_listener_message_received(void *target, EventSourceMessageEventArgs *args) {
    assert(target);
    assert(args);

    NotificationListener *listener = (NotificationListener *) target;

    List *handlers;
    if (hashtable_get(listener->handlers, (void *) args->event, (void **) &handlers) == CC_OK) {
        assert(handlers);
        LIST_FOREACH(item, handlers, {
            NotificationListenerEventHandler *handler = (NotificationListenerEventHandler *) item;
            NotificationListenerEvent *event = notification_listener_event_create(args->event, args->message);
            handler->handler(handler->target, event);
            notification_listener_event_free(event);
        })
    }
}

NotificationListener *ROX_INTERNAL notification_listener_create(NotificationListenerConfig *config) {
    assert(config);
    assert(config->listen_url);
    assert(config->app_key);
    NotificationListener *listener = calloc(1, sizeof(NotificationListener));
    listener->listen_url = mem_copy_str(config->listen_url);
    listener->app_key = mem_copy_str(config->app_key);
    hashtable_new(&listener->handlers);

    listener->testing = config->testing;
    listener->current_thread = config->current_thread;

    if (config->testing) {
        listener->reader = _event_source_reader_create(
                "test", listener, &_notification_listener_message_received);
    }

    return listener;
}

void ROX_INTERNAL notification_listener_on(
        NotificationListener *listener,
        const char *event_name,
        void *target,
        notification_listener_event_handler handler) {

    assert(listener);
    assert(event_name);
    assert(handler);

    void *key = mem_copy_str(event_name);
    if (!hashtable_contains_key(listener->handlers, key)) {
        List *handlers;
        list_new(&handlers);
        hashtable_add(listener->handlers, key, handlers);
    }

    NotificationListenerEventHandler *h = malloc(sizeof(NotificationListenerEventHandler));
    h->target = target;
    h->handler = handler;

    List *handlers;
    if (hashtable_get(listener->handlers, key, (void **) &handlers) == CC_OK) {
        assert(handlers);
        list_add(handlers, h);
    } else {
        assert(false);
    }
}

void ROX_INTERNAL notification_listener_test(NotificationListener *listener, const char *input) {
    assert(listener);
    assert(input);
    assert(listener->testing);
    assert(listener->reader);
    _event_source_reader_write_callback((char *) input, 1, strlen(input), listener->reader);
}

void ROX_INTERNAL notification_listener_start(NotificationListener *listener) {
    assert(listener);
    if (listener->testing || listener->reader) {
        return;
    }
    char *url = mem_build_url(listener->listen_url, listener->app_key);
    listener->reader = _event_source_reader_create(
            url, listener, &_notification_listener_message_received);
    _event_source_reader_start(listener->reader, listener->current_thread);
    free(url);
}

void ROX_INTERNAL notification_listener_stop(NotificationListener *listener) {
    assert(listener);
    if (!listener->testing && listener->reader) {
        if (listener->reader->reading) {
            _event_source_reader_stop(listener->reader);
        }
        _event_source_reader_free(listener->reader);
        listener->reader = NULL;
    }
}

void ROX_INTERNAL notification_listener_free(NotificationListener *listener) {
    assert(listener);
    free(listener->listen_url);
    free(listener->app_key);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, listener->handlers, {
        free(entry->key);
        list_destroy_cb(entry->value, &free);
    })
    hashtable_destroy(listener->handlers);
    if (listener->reader) {
        _event_source_reader_free(listener->reader);
    }
    free(listener);
}
