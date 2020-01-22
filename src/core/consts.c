#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "core/consts.h"
#include "util.h"

//
// Build
//

static const char *ROX_INTERNAL ROX_PLATFORM = "C";
static const char *ROX_INTERNAL ROX_API_VERSION = "1.8.0";

//
// Environment
//

void ROX_INTERNAL rox_env_get_internal_path(char *buffer, int buffer_size) {
    assert(buffer);
    str_copy_value_to_buffer(buffer, buffer_size, "device/request_configuration");
}

#define ROX_ENV_VAL_BUFFER_SIZE 1024

void ROX_INTERNAL rox_env_return_value_using_mode_env(
        const char *buffer,
        int buffer_size,
        const char *local_mode_value,
        const char *qa_mode_value,
        const char *prod_mode_value) {

    assert(buffer);
    assert(buffer_size > 0);
    char value[ROX_ENV_VAL_BUFFER_SIZE];
    size_t len;
    if (getenv_s(&len, value, ROX_ENV_VAL_BUFFER_SIZE, "ROLLOUT_MODE") != 0 && len > 0) {
        if (strcmp(value, "QA") == 0) {
            str_copy_value_to_buffer(buffer, buffer_size, qa_mode_value);
        } else if (strcmp(value, "LOCAL") == 0) {
            str_copy_value_to_buffer(buffer, buffer_size, local_mode_value);
        }
    }
    str_copy_value_to_buffer(buffer, buffer_size, prod_mode_value);
}

void ROX_INTERNAL rox_env_get_cdn_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "https://development-conf.rollout.io",
            "https://qa-conf.rollout.io",
            "https://conf.rollout.io");
}

void ROX_INTERNAL rox_env_get_api_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8557/device/get_configuration",
            "https://qax.rollout.io/device/get_configuration",
            "https://x-api.rollout.io/device/get_configuration");
}

void ROX_INTERNAL rox_env_get_state_cdn_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "https://development-statestore.rollout.io",
            "https://qa-statestore.rollout.io",
            "https://statestore.rollout.io");
}

void ROX_INTERNAL rox_env_get_state_api_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8557/device/update_state_store",
            "https://qax.rollout.io/device/update_state_store",
            "https://x-api.rollout.io/device/update_state_store");
}

void ROX_INTERNAL rox_env_get_analytics_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8787",
            "https://qaanalytic.rollout.io",
            "https://analytic.rollout.io");
}

void ROX_INTERNAL rox_env_get_notifications_path(const char *buffer, int buffer_size) {
    assert(buffer);
    assert(buffer_size > 0);
    rox_env_return_value_using_mode_env(
            buffer, buffer_size,
            "http://127.0.0.1:8887/sse",
            "https://qax-push.rollout.io/sse",
            "https://push.rollout.io/sse");
}

//
// PropertyType
//

static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_CACHE_MISS_RELATIVE_URL = {1, "cache_miss_relative_url"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_LIB_VERSION = {4, "lib_version"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_ROLLOUT_BUILD = {5, "rollout_build"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_API_VERSION = {6, "api_version"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_BUID = {7, "buid"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_BUID_GENERATORS_LIST = {8, "buid_generators_list"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_APP_RELEASE = {10, "app_release"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_DISTINCT_ID = {11, "distinct_id"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_APP_KEY = {12, "app_key"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_FEATURE_FLAGS = {13, "feature_flags"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_REMOTE_VARIABLES = {14, "remote_variables"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES = {15, "custom_properties"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_PLATFORM = {16, "platform"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_DEV_MODE_SECRET = {17, "devModeSecret"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_STATE_MD5 = {18, "state_md5"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_FEATURE_FLAGS_STRING = {19, "feature_flags_string"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_REMOTE_VARIABLES_STRING = {20, "remote_variables_string"};
static const PropertyType ROX_INTERNAL ROX_PROPERTY_TYPE_CUSTOM_PROPERTIES_STRING = {21, "custom_properties_string"};
