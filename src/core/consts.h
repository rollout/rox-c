#pragma once

#include "roxapi.h"

//
// Build
//

extern const char *ROX_INTERNAL ROX_PLATFORM;
extern const char *ROX_INTERNAL ROX_API_VERSION;

//
// Environment
//

void ROX_INTERNAL rox_env_get_internal_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_cdn_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_api_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_state_cdn_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_state_api_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_analytics_path(char* buffer, int buffer_size);

void ROX_INTERNAL rox_env_get_notifications_path(char* buffer, int buffer_size);

//
// PropertyType
//

typedef struct ROX_INTERNAL PropertyType {
    const int value;
    const char *const name;
} PropertyType;

extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_LIB_VERSION;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_ROLLOUT_BUILD;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_API_VERSION;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_BUID;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_BUID_GENERATORS_LIST;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_APP_RELEASE;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_DISTINCT_ID;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_APP_KEY;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_FEATURE_FLAGS;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_REMOTE_VARIABLES;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_PLATFORM;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_DEV_MODE_SECRET;
extern const ROX_INTERNAL PropertyType ROX_PROPERTY_TYPE_STATE_MD5;
