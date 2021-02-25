#include <assert.h>
#include "client.h"
#include "consts.h"
#include "util.h"
#include "device.h"
#include "entities.h"
#include "repositories.h"
#include "collections.h"

//
// SdkSettings
//

ROX_INTERNAL SdkSettings *sdk_settings_create(const char *api_key, const char *dev_mode_secret) {
    assert(api_key);
    assert(dev_mode_secret);
    SdkSettings *settings = calloc(1, sizeof(SdkSettings));
    settings->api_key = mem_copy_str(api_key);
    settings->dev_mode_secret = mem_copy_str(dev_mode_secret);
    return settings;
}

ROX_INTERNAL void sdk_settings_free(SdkSettings *sdk_settings) {
    assert(sdk_settings);
    free(sdk_settings->api_key);
    free(sdk_settings->dev_mode_secret);
    free(sdk_settings);
}

//
// RoxOptions
//

struct RoxOptions {
    char *version;
    char *dev_mod_key;
    char *roxy_url;
    int fetch_interval;
    void *impression_handler_target;
    rox_impression_handler impression_handler;
    void *configuration_fetched_target;
    rox_configuration_fetched_handler configuration_fetched_handler;
    void *dynamic_properties_rule_target;
    rox_dynamic_properties_rule dynamic_properties_rule;
    bool cxx;
};

ROX_API RoxOptions *rox_options_create() {
    RoxOptions *options = calloc(1, sizeof(RoxOptions));
    options->dev_mod_key = mem_copy_str("stam");
    options->version = mem_copy_str("0.0");
    options->fetch_interval = 60;
    return options;
}

ROX_API void rox_options_set_dev_mode_key(RoxOptions *options, const char *key) {
    assert(options);
    assert(key);
    free(options->dev_mod_key);
    options->dev_mod_key = mem_copy_str(key);
}

ROX_API void rox_options_set_version(RoxOptions *options, const char *version) {
    assert(options);
    assert(version);
    free(options->version);
    options->version = mem_copy_str(version);
}

ROX_API void rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval) {
    assert(options);
    assert(fetch_interval >= 0);
    if (fetch_interval == 0) {
        options->fetch_interval = 0;
    } else {
        options->fetch_interval = fetch_interval < 30 ? 30 : fetch_interval;
    }
}

ROX_API void rox_options_set_roxy_url(RoxOptions *options, const char *roxy_url) {
    assert(options);
    assert(roxy_url);
    if (options->roxy_url) {
        free(options->version);
    }
    options->roxy_url = mem_copy_str(roxy_url);
}

ROX_API void rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        rox_impression_handler handler) {
    assert(options);
    assert(handler);
    options->impression_handler_target = target;
    options->impression_handler = handler;
}

ROX_API void rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        rox_configuration_fetched_handler handler) {
    assert(options);
    assert(handler);
    options->configuration_fetched_target = target;
    options->configuration_fetched_handler = handler;
}

ROX_API void rox_options_set_dynamic_properties_rule(
        RoxOptions *options,
        void *target,
        rox_dynamic_properties_rule rule) {
    assert(options);
    assert(rule);
    options->dynamic_properties_rule_target = target;
    options->dynamic_properties_rule = rule;
}

ROX_INTERNAL void rox_options_set_cxx(RoxOptions *options) {
    assert(options);
    options->cxx = true;
}

ROX_INTERNAL bool rox_options_is_cxx(RoxOptions *options) {
    assert(options);
    return options->cxx;
}

ROX_INTERNAL char *rox_options_get_dev_mode_key(RoxOptions *options) {
    assert(options);
    return options->dev_mod_key;
}

ROX_INTERNAL char *rox_options_get_version(RoxOptions *options) {
    assert(options);
    return options->version;
}

ROX_INTERNAL int rox_options_get_fetch_interval(RoxOptions *options) {
    assert(options);
    return options->fetch_interval;
}

ROX_INTERNAL const char *rox_options_get_roxy_url(RoxOptions *options) {
    assert(options);
    return options->roxy_url;
}

ROX_INTERNAL rox_impression_handler rox_options_get_impression_handler(RoxOptions *options) {
    assert(options);
    return options->impression_handler;
}

ROX_INTERNAL void *rox_options_get_impression_handler_target(RoxOptions *options) {
    assert(options);
    return options->impression_handler_target;
}

ROX_INTERNAL rox_configuration_fetched_handler rox_options_get_configuration_fetched_handler(RoxOptions *options) {
    assert(options);
    return options->configuration_fetched_handler;
}

ROX_INTERNAL void *rox_options_get_configuration_fetched_handler_target(RoxOptions *options) {
    assert(options);
    return options->configuration_fetched_target;
}

ROX_INTERNAL rox_dynamic_properties_rule rox_options_get_dynamic_properties_rule(RoxOptions *options) {
    assert(options);
    return options->dynamic_properties_rule;
}

ROX_INTERNAL void *rox_options_get_dynamic_properties_rule_target(RoxOptions *options) {
    assert(options);
    return options->dynamic_properties_rule_target;
}

ROX_INTERNAL void rox_options_free(RoxOptions *options) {
    assert(options);
    free(options->dev_mod_key);
    free(options->version);
    if (options->roxy_url) {
        free(options->roxy_url);
    }
    free(options);
}

//
// DeviceProperties
//

struct DeviceProperties {
    SdkSettings *sdk_settings;
    RoxOptions *rox_options;
    RoxMap *map;
    const char *distinct_id;
    char *env;
};

ROX_INTERNAL DeviceProperties *device_properties_create_from_map(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options,
        RoxMap *map) {

    assert(sdk_settings);
    assert(rox_options);
    assert(map);

    DeviceProperties *properties = calloc(1, sizeof(DeviceProperties));
    properties->sdk_settings = sdk_settings;
    properties->rox_options = rox_options;
    properties->map = map;

    const char *value;
    if ((value = getenv(ROX_ENV_MODE_KEY))) {
        if (str_equals(value, ROX_ENV_MODE_QA)) {
            properties->env = mem_copy_str(ROX_ENV_MODE_QA);
        }
        if (str_equals(value, ROX_ENV_MODE_LOCAL)) {
            properties->env = mem_copy_str(ROX_ENV_MODE_LOCAL);
        }
    }
    if (!properties->env) {
        properties->env = mem_copy_str(ROX_ENV_MODE_PRODUCTION);
    }

    properties->distinct_id = rox_globally_unique_device_id();

    return properties;
}

ROX_INTERNAL DeviceProperties *device_properties_create(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options) {
    assert(sdk_settings);
    assert(rox_options);

    RoxMap *map = rox_map_create();
    rox_map_add(map, ROX_PROPERTY_TYPE_LIB_VERSION.name, mem_copy_str(ROX_LIB_VERSION));
    rox_map_add(map, ROX_PROPERTY_TYPE_ROLLOUT_BUILD.name,
                mem_copy_str("50")); // TODO: fix the build number
    rox_map_add(map, ROX_PROPERTY_TYPE_API_VERSION.name, mem_copy_str(ROX_API_VERSION));
    rox_map_add(map, ROX_PROPERTY_TYPE_APP_RELEASE.name,
                mem_copy_str(rox_options_get_version(rox_options))); // used for the version filter
    rox_map_add(map, ROX_PROPERTY_TYPE_DISTINCT_ID.name, mem_copy_str(rox_globally_unique_device_id()));
    rox_map_add(map, ROX_PROPERTY_TYPE_APP_KEY.name, mem_copy_str(sdk_settings->api_key));
    rox_map_add(map, ROX_PROPERTY_TYPE_PLATFORM.name, mem_copy_str(rox_options_is_cxx(rox_options)
                                                                   ? ROX_PLATFORM_CXX : ROX_PLATFORM_C));
    rox_map_add(map, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name,
                mem_copy_str(rox_options_get_dev_mode_key(rox_options)));

    return device_properties_create_from_map(sdk_settings, rox_options, map);
}

ROX_INTERNAL RoxMap *device_properties_get_all_properties(DeviceProperties *properties) {
    assert(properties);
    return properties->map;
}

ROX_INTERNAL const char *device_properties_get_rollout_environment(DeviceProperties *properties) {
    assert(properties);
    return properties->env;
}

ROX_INTERNAL const char *device_properties_get_lib_version(DeviceProperties *properties) {
    assert(properties);
    return ROX_LIB_VERSION;
}

ROX_INTERNAL const char *device_properties_get_distinct_id(DeviceProperties *properties) {
    assert(properties);
    return properties->distinct_id;
}

ROX_INTERNAL const char *device_properties_get_rollout_key(DeviceProperties *properties) {
    assert(properties);
    return properties->sdk_settings->api_key;
}

ROX_INTERNAL void device_properties_free(DeviceProperties *properties) {
    assert(properties);
    free(properties->env);
    rox_map_free_with_values_cb(properties->map, &free);
    free(properties);
}

//
// RoxDynamicApi
//

struct RoxDynamicApi {
    FlagRepository *flag_repository;
    EntitiesProvider *entities_provider;
};

ROX_INTERNAL RoxDynamicApi *dynamic_api_create(
        FlagRepository *flag_repository,
        EntitiesProvider *entities_provider) {
    assert(flag_repository);
    assert(entities_provider);
    RoxDynamicApi *api = calloc(1, sizeof(RoxDynamicApi));
    api->flag_repository = flag_repository;
    api->entities_provider = entities_provider;
    return api;
}

ROX_API bool rox_dynamic_api_is_enabled(
        RoxDynamicApi *api,
        const char *name,
        bool default_value) {
    return rox_dynamic_api_is_enabled_ctx(api, name, default_value, NULL);
}

ROX_API bool rox_dynamic_api_is_enabled_ctx(
        RoxDynamicApi *api,
        const char *name,
        bool default_value,
        RoxContext *context) {
    assert(api);
    assert(name);

    RoxStringBase *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_flag(api->entities_provider, default_value);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }
    return flag_is_enabled_or(variant, context, default_value);
}

ROX_API char *rox_dynamic_api_get_string(
        RoxDynamicApi *api,
        const char *name,
        char *default_value) {
    return rox_dynamic_api_get_string_ctx(api, name, default_value, NULL, NULL);
}

ROX_API char *rox_dynamic_api_get_string_ctx(
        RoxDynamicApi *api,
        const char *name,
        char *default_value,
        RoxList *options,
        RoxContext *context) {

    assert(api);
    assert(name);

    RoxStringBase *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_string(api->entities_provider, default_value, options);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }
    return variant_get_string_or(variant, context, default_value);
}

ROX_API int rox_dynamic_api_get_int(
        RoxDynamicApi *api,
        const char *name,
        int default_value) {
    return rox_dynamic_api_get_int_ctx(api, name, default_value, NULL, NULL);
}

ROX_API int rox_dynamic_api_get_int_ctx(
        RoxDynamicApi *api,
        const char *name,
        int default_value,
        RoxList *options,
        RoxContext *context) {

    assert(api);
    assert(name);

    RoxStringBase *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_int(api->entities_provider, default_value, options);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }
    return variant_get_int_or(variant, context, default_value);
}

ROX_API double rox_dynamic_api_get_double(
        RoxDynamicApi *api,
        const char *name,
        double default_value) {
    return rox_dynamic_api_get_double_ctx(api, name, default_value, NULL, NULL);
}

ROX_API double rox_dynamic_api_get_double_ctx(
        RoxDynamicApi *api,
        const char *name,
        double default_value,
        RoxList *options,
        RoxContext *context) {

    assert(api);
    assert(name);

    RoxStringBase *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_double(api->entities_provider, default_value, options);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }
    return variant_get_double_or(variant, context, default_value);
}

ROX_API void rox_dynamic_api_free(RoxDynamicApi *api) {
    assert(api);
    free(api);
}

//
// InternalFlags
//

struct InternalFlags {
    ExperimentRepository *experiment_repository;
    Parser *parser;
};

ROX_INTERNAL InternalFlags *internal_flags_create(
        ExperimentRepository *experiment_repository,
        Parser *parser) {
    assert(experiment_repository);
    assert(parser);

    InternalFlags *flags = calloc(1, sizeof(InternalFlags));
    flags->experiment_repository = experiment_repository;
    flags->parser = parser;
    return flags;
}

ROX_INTERNAL bool internal_flags_is_enabled(InternalFlags *flags, const char *flag_name) {
    assert(flags);
    assert(flag_name);
    ExperimentModel *internal_experiment = experiment_repository_get_experiment_by_flag(
            flags->experiment_repository, flag_name);
    if (!internal_experiment) {
        return false;
    }
    EvaluationResult *value = parser_evaluate_expression(flags->parser, internal_experiment->condition, NULL);
    char *str_result = result_get_string(value);
    bool enabled = str_result && str_equals(FLAG_TRUE_VALUE, str_result);
    result_free(value);
    return enabled;
}

ROX_INTERNAL int *internal_flags_get_int_value(InternalFlags *flags, const char *flag_name) {
    assert(flags);
    assert(flag_name);
    ExperimentModel *internal_experiment = experiment_repository_get_experiment_by_flag(
            flags->experiment_repository, flag_name);
    if (!internal_experiment) {
        return NULL;
    }
    EvaluationResult *value = parser_evaluate_expression(flags->parser, internal_experiment->condition, NULL);
    int *result = result_get_int(value);
    if (!result) {
        result_free(value);
        return NULL;
    }
    int int_value = *result;
    result_free(value);
    return mem_copy_int(int_value);
}

ROX_INTERNAL void internal_flags_free(InternalFlags *flags) {
    assert(flags);
    free(flags);
}

//
// MD5Generator
//

ROX_INTERNAL char *md5_generator_generate(RoxMap *properties, RoxList *generator_list, RoxList *extra_values) {
    assert(properties);
    assert(generator_list);

    RoxList *values = rox_list_create();
    ROX_LIST_FOREACH(item, generator_list, {
        PropertyType *pt = (PropertyType *) item;
        char *value;
        if (rox_map_get(properties, pt->name, (void **) &value)) {
            rox_list_add(values, value);
        }
    })

    if (extra_values) {
        ROX_LIST_FOREACH(item, extra_values, {
            rox_list_add(values, (char *) item);
        })
    }

    char *concat = mem_str_join("|", values);
    char *result = mem_md5_str(concat);
    rox_list_free(values);
    free(concat);

    str_to_upper(result);
    return result;
}

//
// BUID
//

struct BUID {
    DeviceProperties *device_properties;
    char *buid;
};

ROX_INTERNAL BUID *buid_create(DeviceProperties *device_properties) {
    assert(device_properties);
    BUID *buid = calloc(1, sizeof(BUID));
    buid->device_properties = device_properties;
    return buid;
}

ROX_INTERNAL BUID *buid_create_dummy(const char *value) {
    assert(value);
    BUID *buid = calloc(1, sizeof(BUID));
    buid->buid = mem_copy_str(value);
    return buid;
}

ROX_INTERNAL char *buid_get_value(BUID *buid) {
    assert(buid);

    if (buid->buid) {
        return buid->buid;
    }

    RoxList *buid_generators = rox_list_create();
    rox_list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_PLATFORM);
    rox_list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_APP_KEY);
    rox_list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_LIB_VERSION);
    rox_list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_API_VERSION);

    RoxMap *properties = device_properties_get_all_properties(buid->device_properties);
    buid->buid = md5_generator_generate(properties, buid_generators, NULL);
    rox_list_free(buid_generators);

    return buid->buid;
}

ROX_INTERNAL void buid_free(BUID *buid) {
    assert(buid);
    if (buid->buid) {
        free(buid->buid);
    }
    free(buid);
}
