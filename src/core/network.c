#include <stdlib.h>
#include <assert.h>
#include "network.h"

ConfigurationFetchResult *ROX_INTERNAL configuration_fetch_result_create(cJSON *data, ConfigurationSource source) {
    assert(data);
    assert(source);
    ConfigurationFetchResult *result = calloc(1, sizeof(ConfigurationFetchResult));
    result->parsed_data = data;
    result->source = source;
    return result;
}

void ROX_INTERNAL configuration_fetch_result_free(ConfigurationFetchResult *result) {
    assert(result);
    cJSON_free(result->parsed_data);
    free(result);
}
