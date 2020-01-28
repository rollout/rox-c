#pragma once

#include <cjson/cJSON.h>
#include "roxapi.h"

typedef enum ROX_INTERNAL ConfigurationSource {
    CDN = 1,
    API,
    Roxy,
    URL
} ConfigurationSource;

typedef struct ROX_INTERNAL ConfigurationFetchResult {
    ConfigurationSource source;
    cJSON *parsed_data;
} ConfigurationFetchResult;

/**
 * @param data Not <code>NULL</code>. The caller is responsible for freeing the given pointer.
 * @return Not <code>NULL</code>.
 */
ConfigurationFetchResult *ROX_INTERNAL configuration_fetch_result_create(cJSON *data, ConfigurationSource source);

/**
 * @param result Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetch_result_free(ConfigurationFetchResult *result);