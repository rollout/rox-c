#pragma once

#include "rox/defs.h"
#include "rox/context.h"
#include "repositories.h"
#include "impression.h"
#include "configuration.h"
#include "properties.h"
#include "eval/parser.h"

//
// SdkSettings
//

typedef void (*sdk_settings_free_extra_func)(void *data);

typedef struct SdkSettings SdkSettings;

/**
 * @param api_key Not <code>NULL</code>. Value is copied internally.
 * @param dev_mode_secret Not <code>NULL</code>. Value is copied internally.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL SdkSettings *sdk_settings_create(const char *api_key, const char *dev_mode_secret);

/**
 * @param settings Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *sdk_settings_get_api_key(SdkSettings *settings);

/**
 * @param settings Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *sdk_settings_get_dev_mode_secret(SdkSettings *settings);

/**
 * @param settings Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 */
ROX_INTERNAL void sdk_settings_add_extra(
        SdkSettings *settings,
        const char *key,
        void *data,
        sdk_settings_free_extra_func free_func);

/**
 * @param settings Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *sdk_settings_get_extra(
        SdkSettings *settings,
        const char *key);

/**
 * @param sdk_settings Not <code>NULL</code>.
 */
ROX_INTERNAL void sdk_settings_free(SdkSettings *sdk_settings);

//
// RoxOptions
//

/**
 * @param options Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_options_set_cxx(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 */
ROX_INTERNAL bool rox_options_is_cxx(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *rox_options_get_dev_mode_key(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *rox_options_get_version(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return Fetch interval or <code>0</code> if not set.
 */
ROX_INTERNAL int rox_options_get_fetch_interval(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL const char *rox_options_get_roxy_url(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL rox_impression_handler rox_options_get_impression_handler(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *rox_options_get_impression_handler_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL rox_configuration_fetched_handler rox_options_get_configuration_fetched_handler(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *rox_options_get_configuration_fetched_handler_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL rox_dynamic_properties_rule rox_options_get_dynamic_properties_rule(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *rox_options_get_dynamic_properties_rule_target(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return disable_signature_verification or <code>true</code> if not set.
 */
ROX_INTERNAL bool rox_options_is_disable_signature_verification(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_options_free(RoxOptions *options);

//
// DeviceProperties
//

typedef struct DeviceProperties DeviceProperties;

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
ROX_INTERNAL DeviceProperties *device_properties_create_from_map(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options,
        RoxMap *map);

/**
 * @param sdk_settings Not <code>NULL</code>.
 * @param rox_options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL DeviceProperties *device_properties_create(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options);

/**
 * The returned hashtable must <em>NOT</em> be freed by the caller. It will be done automatically
 * when calling <code>device_properties_free()</code>.
 *
 * @param properties Not <code>NULL</code>.
 * @return Map <code>char *</code> => <code>char *</code>. Not <code>NULL</code>.
 */
ROX_INTERNAL RoxMap *device_properties_get_all_properties(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 *
 * @param properties Not <code>NULL</code>.
 * @return One of <code>ROX_ENV_MODE_QA</code>, <code>ROX_ENV_MODE_LOCAL</code>, <code>ROX_ENV_MODE_PRODUCTION</code>, or <code>ROX_ENV_MODE_PLATFORM</code>.
 */
ROX_INTERNAL const char *device_properties_get_rollout_environment(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
ROX_INTERNAL const char *device_properties_get_lib_version(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
ROX_INTERNAL const char *device_properties_get_distinct_id(DeviceProperties *properties);

/**
 * The returned value should <em>not</em> be freed.
 * @param properties Not <code>NULL</code>.
 */
ROX_INTERNAL const char *device_properties_get_rollout_key(DeviceProperties *properties);

/**
 * @param properties Not <code>NULL</code>.
 */
ROX_INTERNAL void device_properties_free(DeviceProperties *properties);

//
// RoxDynamicApi
//

/**
 * @param flag_repository Not <code>NULL</code>.
 * @param entities_provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxDynamicApi *dynamic_api_create(
        FlagRepository *flag_repository,
        EntitiesProvider *entities_provider);

//
// InternalFlags
//

typedef struct InternalFlags InternalFlags;

/**
 * @param experiment_repository Not <code>NULL</code>.
 * @param parser Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL InternalFlags *internal_flags_create(
        ExperimentRepository *experiment_repository,
        Parser *parser);

/**
 * @param flags Not <code>NULL</code>.
 * @param flag_name Not <code>NULL</code>.
 */
ROX_INTERNAL bool internal_flags_is_enabled(InternalFlags *flags, const char *flag_name);

/**
 * The returned pointer must be freed by the caller after use.
 *
 * @param flags Not <code>NULL</code>.
 * @param flag_name Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL int *internal_flags_get_int_value(InternalFlags *flags, const char *flag_name);

/**
 * @param flags Not <code>NULL</code>.
 */
ROX_INTERNAL void internal_flags_free(InternalFlags *flags);

//
// MD5Generator
//

/**
 * Note: the returned string must be freed after use.
 *
 * @param properties Map property name (<code>char *</code>) => property value (<code>char *</code>). Not <code>NULL</code>. The caller is responsible for freeing it.
 * @param generator_list List of <code>PropertyType *</code>. Not <code>NULL</code>. The caller is responsible for freeing it.
 * @param extra_values List of <code>char* </code> May be <code>NULL</code>. The caller is responsible for freeing it.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *md5_generator_generate(RoxMap *properties, RoxList *generator_list, RoxList *extra_values);

//
// BUID
//

typedef struct BUID BUID;

/**
 * @param device_properties Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL BUID *buid_create(DeviceProperties *device_properties);

/**
 * @param build Not <code>NULL</code>. Will be coped internally.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL BUID *buid_create_dummy(const char *buid);

/**
 * Note the returned value will be freed when calling <code>buid_free()</code>.
 *
 * @param buid Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *buid_get_value(BUID *buid);

/**
 * @param buid Not <code>NULL</code>.
 */
ROX_INTERNAL void buid_free(BUID *buid);
