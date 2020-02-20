#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <collectc/hashset.h>
#include <collectc/list.h>

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

typedef enum ROX_API RoxLogLevel {
    RoxLogLevelDebug = 1,
    RoxLogLevelWarning,
    RoxLogLevelError,
    RoxLogLevelNone
} RoxLogLevel;

typedef struct ROX_API RoxLogMessage {
    const char *file;
    int line;
    RoxLogLevel level;
    const char *level_name;
    const char *message;
} RoxLogMessage;

typedef void ROX_API (*rox_logging_handler)(void *target, RoxLogMessage *message);

typedef struct ROX_API RoxLoggingConfig {
    RoxLogLevel min_level;
    void *target;
    rox_logging_handler handler;
} RoxLoggingConfig;

void ROX_API rox_logging_init(RoxLoggingConfig *config);

//
// RoxReportingValue
//

typedef struct ROX_API RoxReportingValue {
    const char *name;
    const char *value;
} RoxReportingValue;

// RoxExperiment

typedef struct ROX_API RoxExperiment {
    char *name;
    char *identifier;
    bool archived;
    HashSet *labels;
    char *stickiness_property;
} RoxExperiment;

//
// DynamicValue
//

typedef struct ROX_API RoxDynamicValue RoxDynamicValue;

RoxDynamicValue *ROX_API rox_dynamic_value_create_int(int value);

RoxDynamicValue *ROX_API rox_dynamic_value_create_double(double value);

RoxDynamicValue *ROX_API rox_dynamic_value_create_double_ptr(double *value);

RoxDynamicValue *ROX_API rox_dynamic_value_create_boolean(bool value);

/**
 * Note: the given string will be copied internally.
 * The caller is responsible for freeing it after use.
 */
RoxDynamicValue *ROX_API rox_dynamic_value_create_string_copy(const char *value);

/**
 * Note: the given string will be destroyed in <code>dynamic_value_free()</code>.
 */
RoxDynamicValue *ROX_API rox_dynamic_value_create_string_ptr(char *value);

/**
 * Note: the ownership of the list is delegated to the dynamic value
 * so all the memory will be freed by <code>dynamic_value_free</code>.
 *
 * @param value List of <code>RoxDynamicValue*</code>
 */
RoxDynamicValue *ROX_API rox_dynamic_value_create_list(List *value);

/**
 * Note: the ownership of the map is delegated to the dynamic value
 * so all the memory including both keys and values
 * will be freed by <code>dynamic_value_free</code>.
 *
 * @param value Keys are <code>char *</code>s and values are <code>RoxDynamicValue*</code>s.
 */
RoxDynamicValue *ROX_API rox_dynamic_value_create_map(HashTable *value);

RoxDynamicValue *ROX_API rox_dynamic_value_create_null();

RoxDynamicValue *ROX_API rox_dynamic_value_create_undefined();

RoxDynamicValue *ROX_API rox_dynamic_value_create_copy(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_int(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_double(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_boolean(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_string(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_list(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_map(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_undefined(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_is_null(RoxDynamicValue *value);

int ROX_API rox_dynamic_value_get_int(RoxDynamicValue *value);

double ROX_API rox_dynamic_value_get_double(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_get_boolean(RoxDynamicValue *value);

char *ROX_API rox_dynamic_value_get_string(RoxDynamicValue *value);

List *ROX_API rox_dynamic_value_get_list(RoxDynamicValue *value);

HashTable *ROX_API rox_dynamic_value_get_map(RoxDynamicValue *value);

bool ROX_API rox_dynamic_value_equals(RoxDynamicValue *v1, RoxDynamicValue *v2);

RoxDynamicValue *ROX_API rox_dynamic_value_free(RoxDynamicValue *value);

//
// Context
//

typedef struct ROX_API RoxContext RoxContext;

typedef RoxDynamicValue *ROX_API (*rox_context_get_value_func)(void *target, const char *key);

typedef void *ROX_API (*rox_context_free_target_func)(void *target);

/**
 * @return Not <code>NULL</code>.
 */
RoxContext *ROX_API rox_context_create_empty();

/**
 * Creates context from the given hashtable. The ownership on the given hash table,
 * including its keys and values, is delegated to the created context,
 * and all of the memory will be freed in <code>rox_context_free()</code>.
 *
 * @param map Not <code>NULL</code>. Keys are strings, values are <code>RoxDynamicValue *</code>.
 * @return Not <code>NULL</code>.
 */
RoxContext *ROX_API rox_context_create_from_map(HashTable *map);

/**
 * The called holds the ownership on the given contexts. They will <em>NOT</em> be freed when
 * the returned <code>context</code> is destroyed.
 *
 * @param global_context May be <code>NULL</code>.
 * @param local_context May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
RoxContext *ROX_API rox_context_create_merged(RoxContext *global_context, RoxContext *local_context);

typedef struct ROX_API RoxContextConfig {
    void *target;
    rox_context_get_value_func get_value_func;
    rox_context_free_target_func fee_target_func;
} RoxContextConfig;

/**
 * @param config Not <code>NULL</code>.
 */
RoxContext *ROX_API rox_context_create_custom(RoxContextConfig *config);

/**
 * @param context Not <code>NULL</code>.
 */
void ROX_API rox_context_free(RoxContext *context);

/**
 * @param context Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
RoxDynamicValue *ROX_API rox_context_get(RoxContext *context, const char *key);

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
typedef void ROX_API (*rox_impression_handler)(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context);

//
// ConfigurationFetchedArgs
//

typedef enum ROX_API RoxFetchStatus {
    AppliedFromEmbedded = 1,
    AppliedFromLocalStorage,
    AppliedFromNetwork,
    ErrorFetchedFailed
} RoxFetchStatus;

typedef enum ROX_API RoxFetcherError {
    CorruptedJson = 1,
    EmptyJson,
    SignatureVerificationError,
    NetworkError,
    MismatchAppKey,
    UnknownError,
    NoError
} RoxFetcherError;

typedef struct ROX_API RoxConfigurationFetchedArgs {
    RoxFetchStatus fetcher_status;
    const char *creation_date;
    bool has_changes;
    RoxFetcherError error_details;
} RoxConfigurationFetchedArgs;

typedef void ROX_API (*rox_configuration_fetched_handler)(void *target, RoxConfigurationFetchedArgs *args);

//
// DynamicPropertiesRule
//

/**
 * Note the returned value, if not <code>NULL</code>, must be freed by the caller
 * by invoking <code>dynamic_value_free()</code>
 */
typedef ROX_API RoxDynamicValue *(*rox_dynamic_properties_rule)(
        const char *prop_name,
        void *target,
        RoxContext *context);

//
// Options
//

typedef struct ROX_API RoxOptions RoxOptions;

RoxOptions *ROX_API rox_options_create();

/**
 * The caller is responsible for freeing the passed <code>key</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_options_set_dev_mode_key(RoxOptions *options, const char *key);

/**
 * The caller is responsible for freeing the passed <code>version</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param version Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_options_set_version(RoxOptions *options, const char *version);

/**
 * @param options Not <code>NULL</code>.
 * @param fetch_interval Interval in seconds. Should be not less than 30. Otherwise, 30 will be used.
 */
void ROX_API rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval);

/**
 * The caller is responsible for freeing the passed <code>roxy_url</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param roxy_url Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_options_set_roxy_url(
        RoxOptions *options,
        const char *roxy_url);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_API rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        rox_impression_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_API rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        rox_configuration_fetched_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
void ROX_API rox_options_set_dynamic_properties_rule(
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
void ROX_API rox_setup(const char *api_key, RoxOptions *options);

/**
 * @param context Not <code>NULL</code>. The ownership is delegated to ROX.
 */
void ROX_API rox_set_context(RoxContext *context);

void ROX_API rox_fetch();

typedef struct ROX_API RoxVariant RoxVariant;

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 */
RoxVariant *ROX_API rox_add_flag(const char *name, bool default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 */
RoxVariant *ROX_API rox_add_variant(const char *name, const char *default_value, List *options);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if the value is not defined.
 */
char *ROX_API rox_variant_get_value_or_default(RoxVariant *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>NULL</code>, if the value is not defined.
 */
char *ROX_API rox_variant_get_value_or_null(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
bool ROX_API rox_flag_is_enabled(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return <code>true</code> or <code>false</code> or <code>NULL</code>.
 */
const bool *ROX_API rox_flag_is_enabled_or_null(RoxVariant *variant);

typedef ROX_API void (*rox_flag_action)();

/**
 * @param variant Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
void ROX_API rox_flag_enabled_do(RoxVariant *variant, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
void ROX_API rox_flag_disabled_do(RoxVariant *variant, rox_flag_action action);

typedef ROX_API RoxDynamicValue *(*rox_custom_property_value_generator)(void *target, RoxContext *context);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_set_custom_string_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
void ROX_API rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_set_custom_boolean_property(const char *name, bool value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
void ROX_API rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_set_custom_double_property(const char *name, double value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
void ROX_API rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_set_custom_integer_property(const char *name, int value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
void ROX_API rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
void ROX_API rox_set_custom_semver_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
void ROX_API rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

//
// DynamicApi
//

typedef struct ROX_API RoxDynamicApi RoxDynamicApi;

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
char *ROX_API rox_dynamic_api_get_value(
        RoxDynamicApi *api,
        const char *name,
        char *default_value,
        List *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 */
void ROX_API rox_dynamic_api_free(RoxDynamicApi *api);

/**
 * Note the returned pointer must be freed after use by calling <code>rox_dynamic_api_free</code>.
 *
 * @return Not <code>NULL</code>.
 */
RoxDynamicApi *ROX_API rox_dynamic_api();

/**
 * Should be called to free all the rox memory. After this,
 * no <code>rox_xxx</code> method can be called anymore.
 */
void ROX_API rox_shutdown();

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

// TODO: put API class(es) declaration here

#endif