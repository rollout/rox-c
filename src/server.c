#include <assert.h>
#include <core/consts.h>
#include "core/logging.h"
#include "core.h"

typedef struct ROX_INTERNAL Rox {
    RoxCore *core;
    RoxContext *global_context;
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    RoxOptions *options;
} Rox;

static Rox *rox = NULL;

static void _create_custom_property(
        HashTable *map,
        const char *name,
        const char *value,
        const char *suffix,
        const PropertyType *type,
        const CustomPropertyType *property_type) {

    assert(map);

    if (!value) {
        hashtable_get(map, type->name, (void **) &value);
    }

    rox_core_add_custom_property_if_not_exists(
            rox->core,
            device_property_create_using_value(
                    name ? name : type->name,
                    property_type,
                    value
                    ? rox_dynamic_value_create_string_copy(value)
                    : rox_dynamic_value_create_null()));
}

void ROX_API rox_setup(const char *api_key, RoxOptions *options) {
    assert(api_key);

    if (rox) {
        ROX_ERROR("Calling rox_setup more than once");
        return;
    }

    rox = calloc(1, sizeof(Rox));
    rox->core = rox_core_create();

    if (!options) {
        options = rox_options_create();
    }

    rox->options = options;
    rox->sdk_settings = sdk_settings_create(api_key, rox_options_get_dev_mode_key(options));
    rox->device_properties = device_properties_create(rox->sdk_settings, options);

    HashTable *props = device_properties_get_all_properties(rox->device_properties);
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
    }
}

static bool _check_setup_called() {
    assert(rox);
    if (!rox) {
        ROX_ERROR("rox_setup is not called");
        return false;
    }
    return true;
}

void ROX_API rox_set_context(RoxContext *context) {
    assert(context);
    if (!_check_setup_called()) {
        return;
    }
    if (rox->global_context) {
        rox_context_free(rox->global_context);
    }
    rox->global_context = context;
    rox_core_set_context(rox->core, context);
}

RoxVariant *ROX_API rox_add_flag(const char *name, bool default_value) {
    assert(name);
    RoxVariant *flag = variant_create_flag_with_default(default_value);
    if (_check_setup_called()) {
        rox_core_add_flag(rox->core, flag, name);
    }
    return flag;
}

RoxVariant *ROX_API rox_add_variant(const char *name, const char *default_value, List *options) {
    assert(name);
    RoxVariant *variant = variant_create(default_value, options);
    if (_check_setup_called()) {
        rox_core_add_flag(rox->core, variant, name);
    }
    return variant;
}

char *ROX_API rox_variant_get_value_or_default(RoxVariant *variant) {
    assert(variant);
    return variant_get_value_or_default(variant, NULL);
}

char *ROX_API rox_variant_get_value_or_null(RoxVariant *variant) {
    assert(variant);
    return variant_get_value_or_null(variant, NULL);
}

bool ROX_API rox_flag_is_enabled(RoxVariant *variant) {
    assert(variant);
    return flag_is_enabled(variant, NULL);
}

const bool *ROX_API rox_flag_is_enabled_or_null(RoxVariant *variant) {
    assert(variant);
    return flag_is_enabled_or_null(variant, NULL);
}

void ROX_API rox_flag_enabled_do(RoxVariant *variant, rox_flag_action action) {
    assert(variant);
    assert(action);
    flag_enabled_do(variant, NULL, action);
}

void ROX_API rox_flag_disabled_do(RoxVariant *variant, rox_flag_action action) {
    assert(variant);
    assert(action);
    flag_disabled_do(variant, NULL, action);
}

static void _add_custom_prop(const char *name, const CustomPropertyType *type, void *target,
                             rox_custom_property_value_generator generator) {
    assert(name);
    assert(type);
    assert(generator);
    if (_check_setup_called()) {
        rox_core_add_custom_property(rox->core, custom_property_create(name, type, target, generator));
    }
}

static void _add_custom_prop_value(const char *name, const CustomPropertyType *type, RoxDynamicValue *value) {
    assert(name);
    assert(value);
    if (_check_setup_called()) {
        rox_core_add_custom_property(rox->core, custom_property_create_using_value(name, type, value));
    }
}

void ROX_API rox_set_custom_string_property(const char *name, const char *value) {
    assert(name);
    assert(value);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, rox_dynamic_value_create_string_copy(value));
}

void ROX_API rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, target, generator);
}

void ROX_API rox_set_custom_boolean_property(const char *name, bool value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, rox_dynamic_value_create_boolean(value));
}

void ROX_API rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, target, generator);
}

void ROX_API rox_set_custom_double_property(const char *name, double value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, rox_dynamic_value_create_double(value));
}

void ROX_API rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, target, generator);
}

void ROX_API rox_set_custom_integer_property(const char *name, int value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, rox_dynamic_value_create_int(value));
}

void ROX_API rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, target, generator);
}

void ROX_API rox_set_custom_semver_property(const char *name, const char *value) {
    assert(name);
    _add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, rox_dynamic_value_create_string_copy(value));
}

void ROX_API rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    _add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, target, generator);
}

void ROX_API rox_shutdown() {
    assert(rox);
    if (!_check_setup_called()) {
        return;
    }
    rox_core_free(rox->core);
    if (rox->global_context) {
        rox_context_free(rox->global_context);
    }
    if (rox->sdk_settings) {
        sdk_settings_free(rox->sdk_settings);
    }
    if (rox->device_properties) {
        device_properties_free(rox->device_properties);
    }
    if (rox->options) {
        rox_options_free(rox->options);
    }
    free(rox);
    rox = NULL;
}
