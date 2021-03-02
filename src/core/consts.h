#pragma once

#include "rox/server.h"

//
// Build
//

ROX_INTERNAL extern const char *ROX_PLATFORM_C;
ROX_INTERNAL extern const char *ROX_PLATFORM_CXX;
ROX_INTERNAL extern const char *ROX_API_VERSION;
ROX_INTERNAL extern const char *ROX_ENV_MODE_KEY;
ROX_INTERNAL extern const char *ROX_ENV_MODE_QA;
ROX_INTERNAL extern const char *ROX_ENV_MODE_LOCAL;
ROX_INTERNAL extern const char *ROX_ENV_MODE_PRODUCTION;

//
// Environment
//

ROX_INTERNAL size_t rox_env_get_cdn_path(char *buffer, size_t buffer_size);

ROX_INTERNAL size_t rox_env_get_api_path(char *buffer, size_t buffer_size);

ROX_INTERNAL size_t rox_env_get_state_cdn_path(char *buffer, size_t buffer_size);

ROX_INTERNAL size_t rox_env_get_state_api_path(char *buffer, size_t buffer_size);

ROX_INTERNAL size_t rox_env_get_analytics_path(char *buffer, size_t buffer_size);

ROX_INTERNAL size_t rox_env_get_notifications_path(char *buffer, size_t buffer_size);

//
// PropertyType
//

typedef struct PropertyType {
    const int value;
    char *name;
} PropertyType;

ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_LIB_VERSION;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_API_VERSION;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_BUID;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_BUID_GENERATORS_LIST;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_APP_RELEASE;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_DISTINCT_ID;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_APP_KEY;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_FEATURE_FLAGS;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_REMOTE_VARIABLES;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_PLATFORM;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_DEV_MODE_SECRET;
ROX_INTERNAL extern const PropertyType ROX_PROPERTY_TYPE_STATE_MD5;
