#include <assert.h>
#include <core/consts.h>
#include "core/logging.h"
#include "core.h"
#include "util.h"
#include "collections.h"

typedef struct Rox {
    RoxCore *core;
    RoxContext *global_context;
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    EntitiesProvider *entities_provider;
    RoxOptions *options;
    bool initialized;
} Rox;

static Rox *_rox = NULL;

static Rox *_rox_get_or_create() {
    if (!_rox) {
        _rox = calloc(1, sizeof(Rox));
        _rox->core = rox_core_create(NULL);
    }
    return _rox;
}

static void _create_custom_property(
        RoxMap *map,
        const char *name,
        const char *value,
        const char *prefix,
        const PropertyType *type,
        const CustomPropertyType *property_type) {

    assert(map);

    if (!value) {
        rox_map_get(map, type->name, (void **) &value);
    }

    if (!name) {
        name = type->name;
    }

    char *property_name = prefix
                          ? mem_str_format("%s%s", prefix, name)
                          : mem_copy_str(name);

    Rox *rox = _rox_get_or_create();
    rox_core_add_custom_property_if_not_exists(
            rox->core,
            device_property_create_using_value(
                    property_name,
                    property_type,
                    value
                    ? rox_dynamic_value_create_string_copy(value)
                    : rox_dynamic_value_create_null()));

    free(property_name);
}

ROX_API void rox_setup(const char *api_key, RoxOptions *options) {
    assert(api_key);

    if (_rox && _rox->initialized) {
        ROX_ERROR("Calling rox_setup more than once");
        return;
    }

    Rox *rox = _rox_get_or_create();

    if (!options) {
        options = rox_options_create();
    }

    rox->options = options;
    rox->sdk_settings = sdk_settings_create(api_key, rox_options_get_dev_mode_key(options));
    rox->device_properties = device_properties_create(rox->sdk_settings, options);
    rox->entities_provider = entities_provider_create();

    RoxMap *props = device_properties_get_all_properties(rox->device_properties);
    _create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM, &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    _create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_APP_RELEASE, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    _create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_DISTINCT_ID, &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    _create_custom_property(props, "internal.realPlatform", NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM,
                            &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    _create_custom_property(props, "internal.customPlatform", NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM,
                            &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    _create_custom_property(props, "internal.appKey", api_key, NULL, &ROX_PROPERTY_TYPE_APP_KEY,
                            &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    _create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_LIB_VERSION,
                            &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    _create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_API_VERSION,
                            &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    _create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_DISTINCT_ID,
                            &ROX_CUSTOM_PROPERTY_TYPE_STRING);

    if (!rox_core_setup(rox->core, rox->sdk_settings, rox->device_properties, options)) {
        ROX_ERROR("Failed in rox_setup");
    } else {
        rox->initialized = true;
    }
}

static bool _check_setup_called() {
    assert(_rox);
    assert(_rox->initialized);
    if (!_rox || !_rox->initialized) {
        ROX_ERROR("rox_setup is not called");
        return false;
    }
    return true;
}

ROX_API void rox_fetch() {
    if (!_check_setup_called()) {
        return;
    }
    rox_core_fetch(_rox->core, false);
}

ROX_API void rox_set_context(RoxContext *context) {
    assert(context);
    if (!_check_setup_called()) {
        return;
    }
    if (_rox->global_context) {
        rox_context_free(_rox->global_context);
    }
    _rox->global_context = context;
    rox_core_set_context(_rox->core, context);
}

ROX_API RoxVariant *rox_add_flag(const char *name, bool default_value) {
    assert(name);
    RoxVariant *flag = variant_create_flag_with_default(default_value);
    Rox *rox = _rox_get_or_create();
    rox_core_add_flag(rox->core, flag, name);
    return flag;
}

ROX_API RoxVariant *rox_add_variant(const char *name, const char *default_value, RoxList *options) {
    assert(name);
    RoxVariant *variant = variant_create(default_value, options);
    Rox *rox = _rox_get_or_create();
    rox_core_add_flag(rox->core, variant, name);
    return variant;
}

ROX_API char *rox_variant_get_value_or_default(RoxVariant *variant) {
    assert(variant);
    return variant_get_value_or_default(variant, NULL);
}

ROX_API char *rox_variant_get_value_or_default_ctx(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    return variant_get_value_or_default(variant, context);
}

ROX_API char *rox_variant_get_value_or_null(RoxVariant *variant) {
    assert(variant);
    return variant_get_value_or_null(variant, NULL);
}

ROX_API char *rox_variant_get_value_or_null_ctx(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    return variant_get_value_or_null(variant, context);
}

ROX_API bool rox_flag_is_enabled(RoxVariant *variant) {
    assert(variant);
    return flag_is_enabled(variant, NULL);
}

ROX_API bool rox_flag_is_enabled_ctx(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    return flag_is_enabled(variant, context);
}

ROX_API const bool *rox_flag_is_enabled_or_null(RoxVariant *variant) {
    assert(variant);
    return flag_is_enabled_or_null(variant, NULL);
}

ROX_API const bool *rox_flag_is_enabled_or_null_ctx(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    return flag_is_enabled_or_null(variant, context);
}

ROX_API void rox_flag_enabled_do(RoxVariant *variant, rox_flag_action action) {
    assert(variant);
    assert(action);
    flag_enabled_do(variant, NULL, action);
}

ROX_API void rox_flag_enabled_do_ctx(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    assert(context);
    flag_enabled_do(variant, context, action);
}

ROX_API void rox_flag_disabled_do(RoxVariant *variant, rox_flag_action action) {
    assert(variant);
    assert(action);
    flag_disabled_do(variant, NULL, action);
}

ROX_API void rox_flag_disabled_do_ctx(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    assert(context);
    flag_disabled_do(variant, context, action);
}

static void _add_custom_prop(const char *name, const CustomPropertyType *type, void *target,
                             rox_custom_property_value_generator generator) {
    assert(name);
    assert(type);
    assert(generator);
    Rox *rox = _rox_get_or_create();
    rox_core_add_custom_property(rox->core, custom_property_create(name, type, target, generator));
}

static void _add_custom_prop_value(const char *name, const CustomPropertyType *type, RoxDynamicValue *value) {
    assert(name);
    assert(value);
    Rox *rox = _rox_get_or_create();
    rox_core_add_custom_property(rox->core, custom_property_create_using_value(name, type, value));
}

ROX_API void rox_set_custom_string_property(const char *name, const char *value) {
    assert(name);
    assert(value);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, rox_dynamic_value_create_string_copy(value));
}

ROX_API void rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, target, generator);
}

ROX_API void rox_set_custom_boolean_property(const char *name, bool value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, rox_dynamic_value_create_boolean(value));
}

ROX_API void rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, target, generator);
}

ROX_API void rox_set_custom_double_property(const char *name, double value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, rox_dynamic_value_create_double(value));
}

ROX_API void rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, target, generator);
}

ROX_API void rox_set_custom_integer_property(const char *name, int value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, rox_dynamic_value_create_int(value));
}

ROX_API void rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, target, generator);
}

ROX_API void rox_set_custom_semver_property(const char *name, const char *value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, rox_dynamic_value_create_string_copy(value));
}

ROX_API void rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, target, generator);
}

ROX_API void rox_shutdown() {
    if (!_rox) {
        return;
    }
    rox_core_free(_rox->core);
    if (_rox->global_context) {
        rox_context_free(_rox->global_context);
    }
    if (_rox->sdk_settings) {
        sdk_settings_free(_rox->sdk_settings);
    }
    if (_rox->device_properties) {
        device_properties_free(_rox->device_properties);
    }
    if (_rox->options) {
        rox_options_free(_rox->options);
    }
    if (_rox->entities_provider) {
        entities_provider_free(_rox->entities_provider);
    }
    free(_rox);
    _rox = NULL;
}

ROX_API RoxDynamicApi *rox_dynamic_api() {
    if (!_check_setup_called()) {
        return NULL;
    }
    return rox_core_create_dynamic_api(_rox->core, _rox->entities_provider);
}
