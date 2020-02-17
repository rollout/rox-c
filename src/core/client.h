#pragma once

#include <collectc/hashtable.h>
#include <collectc/list.h>
#include "roxapi.h"
#include "context.h"
#include "repositories.h"
#include "impression.h"
#include "configuration.h"
#include "properties.h"
#include "roxx/parser.h"

//
// SdkSettings
//

typedef struct ROX_INTERNAL SdkSettings {
    char *api_key;
    char *dev_mode_secret;
} SdkSettings;

/**
 * @param api_key Not <code>NULL</code>. Value is copied internally.
 * @param dev_mode_secret Not <code>NULL</code>. Value is copied internally.
 * @return Not <code>NULL</code>.
 */
SdkSettings *ROX_INTERNAL sdk_settings_create(const char *api_key, const char *dev_mode_secret);

/**
 * @param sdk_settings Not <code>NULL</code>.
 */
void ROX_INTERNAL sdk_settings_free(SdkSettings *sdk_settings);

//
// RoxOptions
//

typedef struct ROX_INTERNAL RoxOptions RoxOptions;

RoxOptions *ROX_INTERNAL rox_options_create();

/**
 * The caller is responsible for freeing the passed <code>key</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>. Value is copied internally.
 */
void ROX_INTERNAL rox_options_set_dev_mode_key(RoxOptions *options, const char *key);

/**
 * The caller is responsible for freeing the passed <code>version</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param version Not <code>NULL</code>. Value is copied internally.
 */
void ROX_INTERNAL rox_options_set_version(RoxOptions *options, const char *version);

/**
 * @param options Not <code>NULL</code>.
 * @param fetch_interval Interval in seconds. Should be not less than 30. Otherwise, 30 will be used.
 */
void ROX_INTERNAL rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval);

/**
 * The caller is responsible for freeing the passed <code>roxy_url</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param roxy_url Not <code>NULL</code>. Value is copied internally.
 */
void ROX_INTERNAL rox_options_set_roxy_url(
        RoxOptions *options,
        const char *roxy_url);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        impression_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        configuration_fetched_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_options_set_dynamic_properties_rule(
        RoxOptions *options,
        void *target,
        dynamic_properties_rule rule);

/**
 * @param options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL rox_options_get_dev_mode_key(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL rox_options_get_version(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return Fetch interval or <code>0</code> if not set.
 */
int ROX_INTERNAL rox_options_get_fetch_interval(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
const char *ROX_INTERNAL rox_options_get_roxy_url(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
impression_handler ROX_INTERNAL rox_options_get_impression_handler(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
void *ROX_INTERNAL rox_options_get_impression_handler_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
configuration_fetched_handler ROX_INTERNAL rox_options_get_configuration_fetched_handler(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
void *ROX_INTERNAL rox_options_get_configuration_fetched_handler_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
dynamic_properties_rule ROX_INTERNAL rox_options_get_dynamic_properties_rule(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
void *ROX_INTERNAL rox_options_get_dynamic_properties_rule_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_options_free(RoxOptions *options);

//
// DeviceProperties
//

typedef struct ROX_INTERNAL DeviceProperties DeviceProperties;

/**
 * This function is basically needed for testing.
 *
 * Note the map values should be non-const strings, so they could be freed
 * by calling <code>device_properties_free()</code>. Keys aren't destroyed ever.
 *
 * @param sdk_settings Not <code>NULL</code>.
 * @param rox_options Not <code>NULL</code>.
 * @param map Not <code>NULL</code>. The ownership is delegated to the returned device properties.
 * @return Not <code>NULL</code>.
 */
DeviceProperties *ROX_INTERNAL device_properties_create_from_map(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options,
        HashTable *map);

/**
 * @param sdk_settings Not <code>NULL</code>.
 * @param rox_options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
DeviceProperties *ROX_INTERNAL device_properties_create(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options);

/**
 * The returned hashtable must <em>NOT</em> be freed by the caller. It will be done automatically
 * when calling <code>device_properties_free()</code>.
 *
 * @param properties Not <code>NULL</code>.
 * @return Map <code>char *</code> => <code>char *</code>. Not <code>NULL</code>.
 */
HashTable *ROX_INTERNAL device_properties_get_all_properties(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 *
 * @param properties Not <code>NULL</code>.
 * @return One of <code>ROX_ENV_MODE_QA</code>, <code>ROX_ENV_MODE_LOCAL</code>, or <code>ROX_ENV_MODE_PRODUCTION</code>.
 */
const char *ROX_INTERNAL device_properties_get_rollout_environment(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
const char *ROX_INTERNAL device_properties_get_lib_version(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
const char *ROX_INTERNAL device_properties_get_distinct_id(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
const char *ROX_INTERNAL device_properties_get_rollout_key(DeviceProperties *properties);

/**
 * @param properties Not <code>NULL</code>.
 */
void ROX_INTERNAL device_properties_free(DeviceProperties *properties);

//
// DynamicApi
//

typedef struct ROX_INTERNAL DynamicApi DynamicApi;

/**
 * @param flag_repository Not <code>NULL</code>.
 * @param entities_provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
DynamicApi *ROX_INTERNAL dynamic_api_create(
        FlagRepository *flag_repository,
        EntitiesProvider *entities_provider);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
bool ROX_INTERNAL dynamic_api_is_enabled(
        DynamicApi *api,
        const char *name,
        bool default_value,
        Context *context);

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
char *ROX_INTERNAL dynamic_api_get_value(
        DynamicApi *api,
        const char *name,
        char *default_value,
        List *options,
        Context *context);

/**
 * @param api Not <code>NULL</code>.
 */
void ROX_INTERNAL dynamic_api_free(DynamicApi *api);

//
// InternalFlags
//

typedef struct ROX_INTERNAL InternalFlags InternalFlags;

/**
 * @param experiment_repository Not <code>NULL</code>.
 * @param parser Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
InternalFlags *ROX_INTERNAL internal_flags_create(
        ExperimentRepository *experiment_repository,
        Parser *parser);

/**
 * @param flags Not <code>NULL</code>.
 * @param flag_name Not <code>NULL</code>.
 */
bool ROX_INTERNAL internal_flags_is_enabled(InternalFlags *flags, const char *flag_name);

/**
 * The returned pointer must be freed by the caller after use.
 *
 * @param flags Not <code>NULL</code>.
 * @param flag_name Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
int *ROX_INTERNAL internal_flags_get_int_value(InternalFlags *flags, const char *flag_name);

/**
 * @param flags Not <code>NULL</code>.
 */
void ROX_INTERNAL internal_flags_free(InternalFlags *flags);

//
// MD5Generator
//

/**
 * Note: the returned string must be freed after use.
 *
 * @param properties Map property name (<code>char *</code>) => property value (<code>char *</code>). Not <code>NULL</code>.
 * @param generator_list List of <code>PropertyType *</code>. Not <code>NULL</code>.
 * @param extra_values List of <code>char* </code> May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL md5_generator_generate(HashTable *properties, List *generator_list, List *extra_values);

//
// BUID
//

typedef struct ROX_INTERNAL BUID BUID;

/**
 * @param device_properties Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
BUID *ROX_INTERNAL buid_create(DeviceProperties *device_properties);

/**
 * @param build Not <code>NULL</code>. Will be coped internally.
 * @return Not <code>NULL</code>.
 */
BUID *ROX_INTERNAL buid_create_dummy(const char *buid);

/**
 * Note the returned value will be freed when calling <code>buid_free()</code>.
 *
 * @param buid Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL buid_get_value(BUID *buid);

/**
 * @param buid Not <code>NULL</code>.
 */
void ROX_INTERNAL buid_free(BUID *buid);
