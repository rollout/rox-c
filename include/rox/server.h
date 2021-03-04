#pragma once

#include "rox/api.h"
#include "rox/collections.h"
#include "rox/dynamic.h"

#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#ifdef __cplusplus
extern "C"
{
#endif

//
// Logging
//

typedef enum RoxLogLevel {
    RoxLogLevelTrace = 1,
    RoxLogLevelDebug,
    RoxLogLevelWarning,
    RoxLogLevelError,
    RoxLogLevelNone
} RoxLogLevel;

typedef struct RoxLogMessage {
    const char *file;
    int line;
    RoxLogLevel level;
    const char *level_name;
    const char *message;
} RoxLogMessage;

typedef void (*rox_logging_handler)(void *target, RoxLogMessage *message);

typedef struct RoxLoggingConfig {
    RoxLogLevel min_level;
    void *target;
    rox_logging_handler handler;
    bool print_time;
} RoxLoggingConfig;

#define ROX_LOGGING_CONFIG_INITIALIZER(log_level) {log_level, NULL, NULL, false}

ROX_API void rox_logging_init(RoxLoggingConfig *config);

//
// RoxReportingValue
//

typedef struct RoxReportingValue {
    const char *name;
    const char *value;
} RoxReportingValue;

// RoxExperiment

typedef struct RoxExperiment {
    char *name;
    char *identifier;
    bool archived;
    RoxSet *labels;
    char *stickiness_property;
} RoxExperiment;


//
// Context
//

typedef struct RoxContext RoxContext;

typedef RoxDynamicValue *(*rox_context_get_value_func)(void *target, const char *key);

typedef void *(*rox_context_free_target_func)(void *target);

/**
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_empty();

/**
 * Creates context from the given hashtable. The ownership on the given hash table,
 * including its keys and values, is delegated to the created context,
 * and all of the memory will be freed in <code>rox_context_free()</code>.
 *
 * @param map Not <code>NULL</code>. Keys are strings, values are <code>RoxDynamicValue *</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_from_map(RoxMap *map);

/**
 * The called holds the ownership on the given contexts. They will <em>NOT</em> be freed when
 * the returned <code>context</code> is destroyed.
 *
 * @param global_context May be <code>NULL</code>.
 * @param local_context May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_merged(RoxContext *global_context, RoxContext *local_context);

typedef struct RoxContextConfig {
    void *target;
    rox_context_get_value_func get_value_func;
    rox_context_free_target_func fee_target_func;
} RoxContextConfig;

/**
 * @param config Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_custom(RoxContextConfig *config);

/**
 * @param context Not <code>NULL</code>.
 */
ROX_API void rox_context_free(RoxContext *context);

/**
 * @param context Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>. If returned, the value must be freed by the caller via invoking <code>rox_dynamic_value_free</code>.
 */
ROX_API RoxDynamicValue *rox_context_get(RoxContext *context, const char *key);

/**
 * Note the reporting <code>value</code> may be freed right after call,
 * but its <code>name</code> and <code>value</code> can live longer.
 * It's recommended to make a copy if you need to use it sometimes later.
 *
 * @param target Can be <code>NULL</code>.
 * @param value Can be <code>NULL</code>.
 * @param experiment Can be <code>NULL</code>.
 * @param context Can be <code>NULL</code>.
 */
typedef void (*rox_impression_handler)(
        void *target,
        RoxReportingValue *value,
        RoxContext *context);

//
// ConfigurationFetchedArgs
//

typedef enum RoxFetchStatus {
    AppliedFromEmbedded = 1,
    AppliedFromLocalStorage,
    AppliedFromNetwork,
    ErrorFetchedFailed
} RoxFetchStatus;

typedef enum RoxFetcherError {
    NoError = 0,
    CorruptedJson = 1,
    EmptyJson,
    SignatureVerificationError,
    NetworkError,
    MismatchAppKey,
    UnknownError
} RoxFetcherError;

typedef struct RoxConfigurationFetchedArgs {
    RoxFetchStatus fetcher_status;
    const char *creation_date;
    bool has_changes;
    RoxFetcherError error_details;
} RoxConfigurationFetchedArgs;

typedef void (*rox_configuration_fetched_handler)(void *target, RoxConfigurationFetchedArgs *args);

//
// DynamicPropertiesRule
//

/**
 * Note the returned value, if not <code>NULL</code>, must be freed by the caller
 * by invoking <code>dynamic_value_free()</code>
 */
typedef RoxDynamicValue *(*rox_dynamic_properties_rule)(
        const char *prop_name,
        void *target,
        RoxContext *context);

//
// Options
//

typedef struct RoxOptions RoxOptions;

ROX_API RoxOptions *rox_options_create();

/**
 * The caller is responsible for freeing the passed <code>key</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_dev_mode_key(RoxOptions *options, const char *key);

/**
 * The caller is responsible for freeing the passed <code>version</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param version Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_version(RoxOptions *options, const char *version);

/**
 * @param options Not <code>NULL</code>.
 * @param fetch_interval Interval in seconds. Should be not less than 30. Otherwise, 30 will be used.
 */
ROX_API void rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval);

/**
 * The caller is responsible for freeing the passed <code>roxy_url</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param roxy_url Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_roxy_url(
        RoxOptions *options,
        const char *roxy_url);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_API void rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        rox_impression_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_API void rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        rox_configuration_fetched_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
ROX_API void rox_options_set_dynamic_properties_rule(
        RoxOptions *options,
        void *target,
        rox_dynamic_properties_rule rule);

//
// Rox
//

/**
 * Must be called before any other <code>rox_xxx</code> calls.
 *
 * @param api_key Not <code>NULL</code>.
 * @param options May be <code>NULL</code>. If passed, the ownership is delegated to ROX.
 */
ROX_API void rox_setup(const char *api_key, RoxOptions *options);

/**
 * @param context May be <code>NULL</code>. The ownership is delegated to ROX.
 */
ROX_API void rox_set_context(RoxContext *context);

ROX_API void rox_fetch();

/**
 * Note: the name RoxStringBase was taken for compatibility with other ROX SDKs.
 */
typedef struct RoxStringBase RoxStringBase;

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_flag(const char *name, bool default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string(const char *name, const char *default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string_with_options(const char *name, const char *default_value, RoxList *options);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int(const char *name, int default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int_with_options(const char *name, int default_value, RoxList *options);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double(const char *name, double default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double_with_options(const char *name, double default_value, RoxList *options);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_string()</code>, if the value is not defined.
 */
ROX_API char *rox_get_string(RoxStringBase *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_string()</code>, if the value is not defined.
 */
ROX_API char *rox_get_string_ctx(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled_ctx(RoxStringBase *variant, RoxContext *context);

typedef void (*rox_flag_action)(void *target);

/**
 * @param variant Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do(RoxStringBase *variant, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do_ctx(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do(RoxStringBase *variant, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do_ctx(
        RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_int()</code>, if the value is not defined.
 */
ROX_API int rox_get_int(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_int()</code>, if the value is not defined.
 */
ROX_API int rox_get_int_ctx(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_double()</code>, if the value is not defined.
 */
ROX_API double rox_get_double(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_double()</code>, if the value is not defined.
 */
ROX_API double rox_get_double_ctx(RoxStringBase *variant, RoxContext *context);

typedef RoxDynamicValue *(*rox_custom_property_value_generator)(void *target, RoxContext *context);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_string_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_boolean_property(const char *name, bool value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_double_property(const char *name, double value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_integer_property(const char *name, int value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_semver_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

//
// DynamicApi
//

typedef struct RoxDynamicApi RoxDynamicApi;

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_API bool rox_dynamic_api_is_enabled(
        RoxDynamicApi *api,
        const char *name,
        bool default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_API bool rox_dynamic_api_is_enabled_ctx(
        RoxDynamicApi *api,
        const char *name,
        bool default_value,
        RoxContext *context);

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API char *rox_dynamic_api_get_string(
        RoxDynamicApi *api,
        const char *name,
        char *default_value);

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API char *rox_dynamic_api_get_string_ctx(
        RoxDynamicApi *api,
        const char *name,
        char *default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 */
ROX_API int rox_dynamic_api_get_int(
        RoxDynamicApi *api,
        const char *name,
        int default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API int rox_dynamic_api_get_int_ctx(
        RoxDynamicApi *api,
        const char *name,
        int default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 */
ROX_API double rox_dynamic_api_get_double(
        RoxDynamicApi *api,
        const char *name,
        double default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API double rox_dynamic_api_get_double_ctx(
        RoxDynamicApi *api,
        const char *name,
        double default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 */
ROX_API void rox_dynamic_api_free(RoxDynamicApi *api);

/**
 * Note the returned pointer must be freed after use by calling <code>rox_dynamic_api_free</code>.
 *
 * @return Not <code>NULL</code>.
 */
ROX_API RoxDynamicApi *rox_dynamic_api();

/**
 * Should be called to free all the rox memory. After this,
 * no <code>rox_xxx</code> method can be called anymore.
 */
ROX_API void rox_shutdown();

#ifdef __cplusplus
} // extern "C"
#endif