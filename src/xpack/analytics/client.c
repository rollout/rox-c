#include <assert.h>
#include "client.h"

//
// Client
//

struct ROX_INTERNAL AnalyticsClient {
    char *write_key;
    AnalyticsClientConfig *config;
    void *target;
    analytics_client_track_func track;
};

static void analytics_client_track_impl(void *target, AnalyticsEvent *event) {
    assert(target);
    assert(event);
    AnalyticsClient *client = (AnalyticsClient *) target;
    // TODO: implement!
}

AnalyticsClient *ROX_INTERNAL analytics_client_create(
        const char *write_key,
        AnalyticsClientConfig *config,
        DeviceProperties *properties) {

    assert(write_key);
    assert(config);
    assert(properties);

    AnalyticsClient *client = calloc(1, sizeof(AnalyticsClient));
    client->write_key = mem_copy_str(write_key);
    client->config = config;
    client->target = config->target ? config->target : client;
    client->track = config->track_func ? config->track_func : analytics_client_track_impl;
    // TODO: init other properties like flush handler etc.
    return client;
}

void ROX_INTERNAL analytics_client_track(AnalyticsClient *client, AnalyticsEvent *event) {
    assert(client);
    assert(event);
    client->track(client->target, event);
}

void ROX_INTERNAL analytics_client_free(AnalyticsClient *client) {
    assert(client);
    if (client->write_key) {
        free(client->write_key);
    }
    free(client);
}
