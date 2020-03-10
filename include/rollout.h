#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Generic helper definitions for shared library support.
// (see https://gcc.gnu.org/wiki/Visibility/)
#if defined _WIN32 || defined __CYGWIN__
    #define ROX_HELPER_DLL_IMPORT __declspec(dllimport)
    #define ROX_HELPER_DLL_EXPORT __declspec(dllexport)
    #define ROX_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
    #define ROX_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define ROX_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define ROX_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
    #define ROX_HELPER_DLL_IMPORT
    #define ROX_HELPER_DLL_EXPORT
    #define ROX_HELPER_DLL_LOCAL
#endif // __GNUC__ >= 4
#endif // defined _WIN32 || defined __CYGWIN__

// Now we use the generic helper definitions above to define ROX_API and ROX_INTERNAL.
// ROX_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// ROX_INTERNAL is used for non-api symbols.

#ifdef ROX_DLL // defined if ROX is compiled as a DLL
#ifdef ROX_DLL_EXPORTS // defined if we are building the ROX DLL (instead of using it)
    #define ROX_API ROX_HELPER_DLL_EXPORT
#else
    #define ROX_API ROX_HELPER_DLL_IMPORT
#endif // ROX_DLL_EXPORTS
    #define ROX_INTERNAL ROX_HELPER_DLL_LOCAL
#else // ROX_DLL is not defined: this means ROX is a static lib.
    #define ROX_API
    #define ROX_INTERNAL
#endif // ROX_DLL

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

// Collections

typedef struct RoxMap RoxMap;

typedef struct RoxList RoxList;

typedef struct RoxSet RoxSet;

ROX_API RoxList *rox_list_create_va(void *skip, ...);

ROX_API RoxList *rox_list_create_str_va(void *skip, ...);

ROX_API RoxSet *rox_set_create_va(void *skip, ...);

ROX_API RoxMap *rox_map_create_va(void *skip, ...);

ROX_API char *mem_copy_str(const char *ptr);

#define ROX_LIST(...) rox_list_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_LIST ROX_LIST(NULL)

#define ROX_LIST_COPY_STR(...) rox_list_create_str_va(NULL, __VA_ARGS__, NULL)

#define ROX_SET(...) rox_set_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_SET ROX_SET(NULL)

#define ROX_COPY(str) mem_copy_str(str)

#define ROX_MAP(...) rox_map_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_MAP ROX_MAP(NULL)

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
// DynamicValue
//

typedef struct RoxDynamicValue RoxDynamicValue;

ROX_API RoxDynamicValue *rox_dynamic_value_create_int(int value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_double(double value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_double_ptr(double *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_boolean(bool value);

/**
 * Note: the given string will be copied internally.
 * The caller is responsible for freeing it after use.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_string_copy(const char *value);

/**
 * Note: the given string will be destroyed in <code>dynamic_value_free()</code>.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_string_ptr(char *value);

/**
 * Note: the ownership of the list is delegated to the dynamic value
 * so all the memory will be freed by <code>dynamic_value_free</code>.
 *
 * @param value List of <code>RoxDynamicValue*</code>
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_list(RoxList *value);

/**
 * Note: the ownership of the map is delegated to the dynamic value
 * so all the memory including both keys and values
 * will be freed by <code>dynamic_value_free</code>.
 *
 * @param value Keys are <code>char *</code>s and values are <code>RoxDynamicValue*</code>s.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_map(RoxMap *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_null();

ROX_API RoxDynamicValue *rox_dynamic_value_create_undefined();

ROX_API RoxDynamicValue *rox_dynamic_value_create_copy(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_int(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_double(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_boolean(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_string(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_list(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_map(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_undefined(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_null(RoxDynamicValue *value);

ROX_API int rox_dynamic_value_get_int(RoxDynamicValue *value);

ROX_API double rox_dynamic_value_get_double(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_get_boolean(RoxDynamicValue *value);

ROX_API char *rox_dynamic_value_get_string(RoxDynamicValue *value);

ROX_API RoxList *rox_dynamic_value_get_list(RoxDynamicValue *value);

ROX_API RoxMap *rox_dynamic_value_get_map(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_equals(RoxDynamicValue *v1, RoxDynamicValue *v2);

ROX_API RoxDynamicValue *rox_dynamic_value_free(RoxDynamicValue *value);

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
 * @return May be <code>NULL</code>.
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
        RoxExperiment *experiment,
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
 * @param context Not <code>NULL</code>. The ownership is delegated to ROX.
 */
ROX_API void rox_set_context(RoxContext *context);

ROX_API void rox_fetch();

typedef struct RoxVariant RoxVariant;

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxVariant *rox_add_flag(const char *name, bool default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxVariant *rox_add_variant(const char *name, const char *default_value, RoxList *options);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if the value is not defined.
 */
ROX_API char *rox_variant_get_value_or_default(RoxVariant *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if the value is not defined.
 */
ROX_API char *rox_variant_get_value_or_default_ctx(RoxVariant *variant, RoxContext *context);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>NULL</code>, if the value is not defined.
 */
ROX_API char *rox_variant_get_value_or_null(RoxVariant *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>NULL</code>, if the value is not defined.
 */
ROX_API char *rox_variant_get_value_or_null_ctx(RoxVariant *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled_ctx(RoxVariant *variant, RoxContext *context);

/**
 * Note the returned pointer must <em>NOT</em> be freed.
 *
 * @param variant Not <code>NULL</code>.
 * @return <code>true</code> or <code>false</code> or <code>NULL</code>.
 */
ROX_API const bool *rox_flag_is_enabled_or_null(RoxVariant *variant);

/**
 * Note the returned pointer must <em>NOT</em> be freed.
 *
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return <code>true</code> or <code>false</code> or <code>NULL</code>.
 */
ROX_API const bool *rox_flag_is_enabled_or_null_ctx(RoxVariant *variant, RoxContext *context);

typedef void (*rox_flag_action)();

/**
 * @param variant Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do(RoxVariant *variant, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do_ctx(RoxVariant *variant, RoxContext *context, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do(RoxVariant *variant, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do_ctx(RoxVariant *variant, RoxContext *context, rox_flag_action action);

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
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API char *rox_dynamic_api_get_value(
        RoxDynamicApi *api,
        const char *name,
        char *default_value,
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