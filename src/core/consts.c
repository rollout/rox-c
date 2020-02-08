#include <stdlib.h>
#include <assert.h>
#include "consts.h"
#include "util.h"

//
// Build
//

const char *ROX_INTERNAL ROX_PLATFORM = "C";
const char *ROX_INTERNAL ROX_API_VERSION = "1.8.0";
const char *ROX_INTERNAL ROX_LIB_VERSION = "1.0.0";
const char *ROX_INTERNAL ROX_ENV_MODE_KEY = "ROLLOUT_MODE";
const char *ROX_INTERNAL ROX_ENV_MODE_QA = "QA";
const char *ROX_INTERNAL ROX_ENV_MODE_LOCAL = "LOCAL";
const char *ROX_INTERNAL ROX_ENV_MODE_PRODUCTION = "PRODUCTION";

//
// Environment
//

size_t ROX_INTERNAL rox_env_get_internal_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    return str_copy_value_to_buffer(buffer, buffer_size, "device/request_configuration");
}

#define ROX_ENV_VAL_BUFFER_SIZE 1024

size_t ROX_INTERNAL _rox_env_return_value_using_mode_env(
        char *buffer,
        int buffer_size,
        const char *local_mode_value,
        const char *qa_mode_value,
        const char *prod_mode_value) {

    assert(buffer);
    assert(buffer_size > 0);
    char value[ROX_ENV_VAL_BUFFER_SIZE];
    size_t len;
    if (getenv_s(&len, value, ROX_ENV_VAL_BUFFER_SIZE, ROX_ENV_MODE_KEY) != 0 && len > 0) {
        if (str_equals(value, ROX_ENV_MODE_QA)) {
            return str_copy_value_to_buffer(buffer, buffer_size, qa_mode_value);
        } else if (str_equals(value, ROX_ENV_MODE_LOCAL)) {
            return str_copy_value_to_buffer(buffer, buffer_size, local_mode_value);
        }
    }
    return str_copy_value_to_buffer(buffer, buffer_size, prod_mode_value);
}

#undef ROX_ENV_VAL_BUFFER_SIZE

size_t ROX_INTERNAL rox_env_get_cdn_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "https://development-conf.rollout.io",
            "https://qa-conf.rollout.io",
            "https://conf.rollout.io");
}

size_t ROX_INTERNAL rox_env_get_api_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8557/device/get_configuration",
            "https://qax.rollout.io/device/get_configuration",
            "https://x-api.rollout.io/device/get_configuration");
}

size_t ROX_INTERNAL rox_env_get_state_cdn_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "https://development-statestore.rollout.io",
            "https://qa-statestore.rollout.io",
            "https://statestore.rollout.io");
}

size_t ROX_INTERNAL rox_env_get_state_api_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8557/device/update_state_store",
            "https://qax.rollout.io/device/update_state_store",
            "https://x-api.rollout.io/device/update_state_store");
}

size_t ROX_INTERNAL rox_env_get_analytics_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8787",
            "https://qaanalytic.rollout.io",
            "https://analytic.rollout.io");
}

size_t ROX_INTERNAL rox_env_get_notifications_path(char *buffer, size_t buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    return _rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8887/sse",
            "https://qax-push.rollout.io/sse",
            "https://push.rollout.io/sse");
}

//
// PropertyType
//

const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL = {1, "cache_miss_relative_url"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_LIB_VERSION = {4, "lib_version"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_ROLLOUT_BUILD = {5, "rollout_build"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_API_VERSION = {6, "api_version"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_BUID = {7, "buid"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_BUID_GENERATORS_LIST = {8, "buid_generators_list"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_APP_RELEASE = {10, "app_release"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_DISTINCT_ID = {11, "distinct_id"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_APP_KEY = {12, "app_key"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_FEATURE_FLAGS = {13, "feature_flags"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_REMOTE_VARIABLES = {14, "remote_variables"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES = {15, "custom_properties"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_PLATFORM = {16, "platform"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_DEV_MODE_SECRET = {17, "devModeSecret"};
const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_STATE_MD5 = {18, "state_md5"};
