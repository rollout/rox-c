#include <collectc/hashtable.h>
#include <assert.h>
#include "client.h"
#include "consts.h"
#include "util.h"
#include "device.h"
#include "entities.h"
#include "repositories.h"

//
// RoxOptions
//

struct ROX_INTERNAL RoxOptions {
    char *version;
    char *dev_mod_key;
    char *roxy_url;
    int fetch_interval;
    void *impression_handler_target;
    impression_handler impression_handler;
    void *configuration_fetched_target;
    configuration_fetched_handler configuration_fetched_handler;
    void *dynamic_properties_rule_target;
    dynamic_properties_rule dynamic_properties_rule;
};

RoxOptions *ROX_INTERNAL rox_options_create() {
    RoxOptions *options = calloc(1, sizeof(RoxOptions));
    options->dev_mod_key = mem_copy_str("stam");
    options->version = mem_copy_str("0.0");
    options->fetch_interval = 60;
    return options;
}

void ROX_INTERNAL rox_options_set_dev_mode_key(RoxOptions *options, const char *key) {
    assert(options);
    assert(key);
    free(options->dev_mod_key);
    options->dev_mod_key = mem_copy_str(key);
}

void ROX_INTERNAL rox_options_set_version(RoxOptions *options, const char *version) {
    assert(options);
    assert(version);
    free(options->version);
    options->version = mem_copy_str(version);
}

void ROX_INTERNAL rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval) {
    assert(options);
    options->fetch_interval = fetch_interval < 30 ? 30 : fetch_interval;
}

void ROX_INTERNAL rox_options_set_roxy_url(RoxOptions *options, const char *roxy_url) {
    assert(options);
    assert(roxy_url);
    if (options->roxy_url) {
        free(options->version);
    }
    options->roxy_url = mem_copy_str(roxy_url);
}

void ROX_INTERNAL rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        impression_handler handler) {
    assert(options);
    assert(handler);
    options->impression_handler_target = target;
    options->impression_handler = handler;
}

void ROX_INTERNAL rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        configuration_fetched_handler handler) {
    assert(options);
    assert(handler);
    options->configuration_fetched_target = target;
    options->configuration_fetched_handler = handler;
}

void ROX_INTERNAL rox_options_set_dynamic_properties_rule(
        RoxOptions *options,
        void *target,
        dynamic_properties_rule rule) {
    assert(options);
    assert(rule);
    options->dynamic_properties_rule_target = target;
    options->dynamic_properties_rule = rule;
}

char *ROX_INTERNAL rox_options_get_dev_mode_key(RoxOptions *options) {
    assert(options);
    return options->dev_mod_key;
}

char *ROX_INTERNAL rox_options_get_version(RoxOptions *options) {
    assert(options);
    return options->version;
}

int ROX_INTERNAL rox_options_get_fetch_interval(RoxOptions *options) {
    assert(options);
    return options->fetch_interval;
}

char *ROX_INTERNAL rox_options_get_roxy_url(RoxOptions *options) {
    assert(options);
    return options->roxy_url;
}

impression_handler ROX_INTERNAL rox_options_get_impression_handler(RoxOptions *options) {
    assert(options);
    return options->impression_handler;
}

void *ROX_INTERNAL rox_options_get_impression_handler_target(RoxOptions *options) {
    assert(options);
    return options->impression_handler_target;
}

configuration_fetched_handler ROX_INTERNAL rox_options_get_configuration_fetched_handler(RoxOptions *options) {
    assert(options);
    return options->configuration_fetched_handler;
}

void *ROX_INTERNAL rox_options_get_configuration_fetched_handler_target(RoxOptions *options) {
    assert(options);
    return options->configuration_fetched_target;
}

dynamic_properties_rule ROX_INTERNAL rox_options_get_dynamic_properties_rule(RoxOptions *options) {
    assert(options);
    return options->dynamic_properties_rule;
}

void *ROX_INTERNAL rox_options_get_dynamic_properties_rule_target(RoxOptions *options) {
    assert(options);
    return options->dynamic_properties_rule_target;
}

void ROX_INTERNAL rox_options_free(RoxOptions *options) {
    assert(options);
    free(options->dev_mod_key);
    free(options->version);
    if (options->roxy_url) {
        free(options->roxy_url);
    }
}

//
// DeviceProperties
//

struct ROX_INTERNAL DeviceProperties {
    SdkSettings *sdk_settings;
    RoxOptions *rox_options;
    HashTable *map;
    const char *distinct_id;
    char *env;
};

#define ROX_ENV_VAL_BUFFER_SIZE 1024

DeviceProperties *ROX_INTERNAL device_properties_create_from_map(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options,
        HashTable *map) {

    assert(sdk_settings);
    assert(rox_options);
    assert(map);

    DeviceProperties *properties = calloc(1, sizeof(DeviceProperties));
    properties->sdk_settings = sdk_settings;
    properties->rox_options = rox_options;
    properties->map = map;

    char value[ROX_ENV_VAL_BUFFER_SIZE];
    size_t len;
    if (getenv_s(&len, value, ROX_ENV_VAL_BUFFER_SIZE, ROX_ENV_MODE_KEY) != 0 && len > 0) {
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

DeviceProperties *ROX_INTERNAL device_properties_create(
        SdkSettings *sdk_settings,
        RoxOptions *rox_options) {
    assert(sdk_settings);
    assert(rox_options);

    HashTable *map;
    hashtable_new(&map);
    hashtable_add(map, ROX_PROPERTY_TYPE_LIB_VERSION.name, mem_copy_str(ROX_LIB_VERSION));
    hashtable_add(map, ROX_PROPERTY_TYPE_ROLLOUT_BUILD.name,
                  mem_copy_str("50")); // TODO: fix the build number
    hashtable_add(map, ROX_PROPERTY_TYPE_API_VERSION.name, mem_copy_str(ROX_API_VERSION));
    hashtable_add(map, ROX_PROPERTY_TYPE_APP_RELEASE.name,
                  mem_copy_str(rox_options_get_version(rox_options))); // used for the version filter
    hashtable_add(map, ROX_PROPERTY_TYPE_DISTINCT_ID.name, mem_copy_str(rox_globally_unique_device_id()));
    hashtable_add(map, ROX_PROPERTY_TYPE_APP_KEY.name, mem_copy_str(sdk_settings->api_key));
    hashtable_add(map, ROX_PROPERTY_TYPE_PLATFORM.name, mem_copy_str(ROX_PLATFORM));
    hashtable_add(map, ROX_PROPERTY_TYPE_DEV_MODE_SECRET.name,
                  mem_copy_str(rox_options_get_dev_mode_key(rox_options)));

    return device_properties_create_from_map(sdk_settings, rox_options, map);
}

#undef ROX_ENV_VAL_BUFFER_SIZE

HashTable *ROX_INTERNAL device_properties_get_all_properties(DeviceProperties *properties) {
    assert(properties);
    return properties->map;
}

const char *ROX_INTERNAL device_properties_get_rollout_environment(DeviceProperties *properties) {
    assert(properties);
    return properties->env;
}

const char *ROX_INTERNAL device_properties_get_lib_version(DeviceProperties *properties) {
    assert(properties);
    return ROX_LIB_VERSION;
}

const char *ROX_INTERNAL device_properties_get_distinct_id(DeviceProperties *properties) {
    assert(properties);
    return properties->distinct_id;
}

const char *ROX_INTERNAL device_properties_get_rollout_key(DeviceProperties *properties) {
    assert(properties);
    return properties->sdk_settings->api_key;
}

void ROX_INTERNAL device_properties_free(DeviceProperties *properties) {
    assert(properties);
    free(properties->env);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, properties->map, {
        free(entry->value);
    })
    free(properties);
}

//
// DynamicApi
//

struct ROX_INTERNAL DynamicApi {
    FlagRepository *flag_repository;
    EntitiesProvider *entities_provider;
};

DynamicApi *ROX_INTERNAL dynamic_api_create(
        FlagRepository *flag_repository,
        EntitiesProvider *entities_provider) {
    assert(flag_repository);
    assert(entities_provider);
    DynamicApi *api = calloc(1, sizeof(DynamicApi));
    api->flag_repository = flag_repository;
    api->entities_provider = entities_provider;
    return api;
}

bool ROX_INTERNAL dynamic_api_is_enabled(
        DynamicApi *api,
        const char *name,
        bool default_value,
        Context *context) {
    assert(api);
    assert(name);

    Variant *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_flag(api->entities_provider, default_value);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }

    Variant *flag = variant;
    if (!flag || !flag->is_flag) {
        return default_value;
    }

    const bool *is_enabled = flag_is_enabled_or_null(flag, context);
    return is_enabled ? *is_enabled : default_value;
}

/**
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
        Context *context) {

    assert(api);
    assert(name);

    Variant *variant = flag_repository_get_flag(api->flag_repository, name);
    if (!variant) {
        variant = entities_provider_create_variant(api->entities_provider, default_value, options);
        flag_repository_add_flag(api->flag_repository, variant, name);
    }

    char *value = variant_get_value_or_null(variant, context);
    return value
           ? value
           : default_value
             ? mem_copy_str(default_value)
             : NULL;
}

void ROX_INTERNAL dynamic_api_free(DynamicApi *api) {
    assert(api);
    free(api);
}

//
// InternalFlags
//

struct ROX_INTERNAL InternalFlags {
    ExperimentRepository *experiment_repository;
    Parser *parser;
};

InternalFlags *ROX_INTERNAL internal_flags_create(
        ExperimentRepository *experiment_repository,
        Parser *parser) {
    assert(experiment_repository);
    assert(parser);

    InternalFlags *flags = calloc(1, sizeof(InternalFlags));
    flags->experiment_repository = experiment_repository;
    flags->parser = parser;
    return flags;
}

bool ROX_INTERNAL internal_flags_is_enabled(InternalFlags *flags, const char *flag_name) {
    assert(flags);
    assert(flag_name);
    ExperimentModel *internal_experiment = experiment_repository_get_experiment_by_flag(
            flags->experiment_repository, flag_name);
    if (!internal_experiment) {
        return false;
    }
    EvaluationResult *value = parser_evaluate_expression(flags->parser, internal_experiment->condition, NULL);
    char *str_result = result_get_string(value);
    bool result = str_result && str_equals(FLAG_TRUE_VALUE, str_result);
    result_free(value);
    return result;
}

int *ROX_INTERNAL internal_flags_get_int_value(InternalFlags *flags, const char *flag_name) {
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

void ROX_INTERNAL internal_flags_free(InternalFlags *flags) {
    assert(flags);
    free(flags);
}

//
// MD5Generator
//

char *ROX_INTERNAL md5_generator_generate(HashTable *properties, List *generator_list, List *extra_values) {
    assert(properties);
    assert(generator_list);

    List *values;
    list_new(&values);
    LIST_FOREACH(item, generator_list, {
        PropertyType *pt = (PropertyType *) item;
        char *value;
        if (hashtable_get(properties, pt->name, (void **) &value) == CC_OK) {
            list_add(values, value);
        }
    })

    if (extra_values) {
        LIST_FOREACH(item, extra_values, {
            list_add(values, (char *) item);
        })
    }

    char *concat = mem_str_join("|", values);
    char *result = mem_md5_str(concat);
    list_destroy(values);
    free(concat);

    str_to_upper(result);
    return result;
}

//
// BUID
//

struct ROX_INTERNAL BUID {
    DeviceProperties *device_properties;
    char *buid;
};

BUID *ROX_INTERNAL buid_create(DeviceProperties *device_properties) {
    assert(device_properties);
    BUID *buid = calloc(1, sizeof(BUID));
    buid->device_properties = device_properties;
    return buid;
}

char *ROX_INTERNAL buid_get_value(BUID *buid) {
    assert(buid);

    if (buid->buid) {
        return buid->buid;
    }

    List *buid_generators;
    list_new(&buid_generators);
    list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_PLATFORM);
    list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_APP_KEY);
    list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_LIB_VERSION);
    list_add(buid_generators, (void *) &ROX_PROPERTY_TYPE_API_VERSION);

    HashTable *properties = device_properties_get_all_properties(buid->device_properties);
    buid->buid = md5_generator_generate(properties, buid_generators, NULL);
    list_destroy(buid_generators);

    return buid->buid;
}

HashTable *ROX_INTERNAL buid_get_query_string_parts(BUID *buid) {
    assert(buid);

    char *value = buid_get_value(buid);

    List *generators;
    list_new(&generators);
    list_add(generators, (void *) ROX_PROPERTY_TYPE_PLATFORM.name);
    list_add(generators, (void *) ROX_PROPERTY_TYPE_APP_KEY.name);
    list_add(generators, (void *) ROX_PROPERTY_TYPE_LIB_VERSION.name);
    list_add(generators, (void *) ROX_PROPERTY_TYPE_API_VERSION.name);

    HashTable *result;
    hashtable_new(&result);
    hashtable_add(result, (void *) ROX_PROPERTY_TYPE_BUID.name, value);

    return result;
}

void ROX_INTERNAL buid_free(BUID *buid) {
    assert(buid);
    free(buid);
}