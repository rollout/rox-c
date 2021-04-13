#include <assert.h>
#include <pthread.h>
#include "rox/server.h"
#include "core/consts.h"
#include "core/logging.h"
#include "core.h"
#include "util.h"
#include "server.h"

#ifdef ROX_CLIENT

#include "storage.h"
#include "freeze.h"
#include "overrides.h"

#endif

typedef struct Rox {
    RoxCore *core;
    RoxContext *global_context;
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    EntitiesProvider *entities_provider;
    RoxOptions *options;
    RoxStateCode state;
} Rox;

static Rox *rox_global = NULL;
static pthread_mutex_t startup_shutdown_lock = PTHREAD_MUTEX_INITIALIZER;
static RequestConfig default_request_config = DEFAULT_REQUEST_CONFIG_INITIALIZER;

ROX_INTERNAL void rox_set_default_request_config(RequestConfig *config) {
    if (config) {
        default_request_config = *config;
        return;
    }
    RequestConfig default_config = DEFAULT_REQUEST_CONFIG_INITIALIZER;
    default_request_config = default_config;
}

static Rox *rox_get_or_create() {
    if (!rox_global) {
        rox_global = calloc(1, sizeof(Rox));
        rox_global->core = rox_core_create(&default_request_config);
        rox_global->state = RoxUninitialized;
    }
    return rox_global;
}

static void create_custom_property(
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

    Rox *rox = rox_get_or_create();
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

static bool is_error_state(RoxStateCode state) {
    return state < 0;
}

static void reset_state() {
    rox_global->state = RoxShuttingDown;
#ifdef ROX_CLIENT
    rox_overrides_uninit();
    rox_freeze_uninit();
#endif
    rox_core_free(rox_global->core);
    if (rox_global->global_context) {
        rox_context_free(rox_global->global_context);
    }
    if (rox_global->sdk_settings) {
        sdk_settings_free(rox_global->sdk_settings);
    }
    if (rox_global->device_properties) {
        device_properties_free(rox_global->device_properties);
    }
    if (rox_global->options) {
        rox_options_free(rox_global->options);
    }
    if (rox_global->entities_provider) {
        entities_provider_free(rox_global->entities_provider);
    }
    free(rox_global);
    rox_global = NULL;
}

ROX_API RoxStateCode rox_setup(const char *api_key, RoxOptions *options) {
    assert(api_key);

    pthread_mutex_lock(&startup_shutdown_lock);

    if (rox_global) {
        if ((rox_global->state != RoxUninitialized) && !is_error_state(rox_global->state)) {
            ROX_ERROR("Calling rox_setup more than once");
            pthread_mutex_unlock(&startup_shutdown_lock);
            return rox_global->state;
        }

        if (is_error_state(rox_global->state)) {
            reset_state(); // reset state in case of an error
        }
    }

    if (!options) {
        options = rox_options_create();
    }

    if (!api_key) {
        api_key = "";
    }

    Rox *rox = rox_get_or_create();
    rox->state = RoxSettingUp;
    rox->options = options;
    rox->sdk_settings = sdk_settings_create(api_key, rox_options_get_dev_mode_key(options));
    rox->device_properties = device_properties_create(rox->sdk_settings, options);
    rox->entities_provider = entities_provider_create();

    RoxMap *props = device_properties_get_all_properties(rox->device_properties);
    create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM, &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_APP_RELEASE, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    create_custom_property(props, NULL, NULL, NULL, &ROX_PROPERTY_TYPE_DISTINCT_ID, &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    create_custom_property(props, "internal.realPlatform", NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM,
                           &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    create_custom_property(props, "internal.customPlatform", NULL, NULL, &ROX_PROPERTY_TYPE_PLATFORM,
                           &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    create_custom_property(props, "internal.appKey", api_key, NULL, &ROX_PROPERTY_TYPE_APP_KEY,
                           &ROX_CUSTOM_PROPERTY_TYPE_STRING);
    create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_LIB_VERSION,
                           &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_API_VERSION,
                           &ROX_CUSTOM_PROPERTY_TYPE_SEMVER);
    create_custom_property(props, NULL, NULL, "internal.", &ROX_PROPERTY_TYPE_DISTINCT_ID,
                           &ROX_CUSTOM_PROPERTY_TYPE_STRING);

#ifdef ROX_CLIENT

    RoxStorage *storage = storage_create_from_options(options);
    storage_init(storage, rox->sdk_settings);

    // NOTE: the order is important here because first we must check if the flag value
    // is overridden, and then use the frozen value. If applying in reverse order,
    // it will make flag freeze check first.
    rox_freeze_init(rox->core, options);
    rox_overrides_init(rox->core, storage);
#endif

    rox->state = rox_core_setup(rox->core, rox->sdk_settings, rox->device_properties, options);
    if (is_error_state(rox->state)) {
        ROX_ERROR("Failed in rox_setup; error code is %d", rox->state);
    }

    pthread_mutex_unlock(&startup_shutdown_lock);
    return rox->state;
}

static bool check_setup_called() {
    if (!rox_global || rox_global->state != RoxInitialized) {
        ROX_ERROR("rox_setup is not called");
        return false;
    }
    return true;
}

ROX_API void rox_fetch() {
    if (!check_setup_called()) {
        return;
    }
    rox_core_fetch(rox_global->core, false);
}

ROX_API void rox_set_context(RoxContext *context) {
    if (!check_setup_called()) {
        return;
    }
    if (rox_global->global_context) {
        rox_context_free(rox_global->global_context);
    }
    rox_global->global_context = context;
    rox_core_set_context(rox_global->core, context);
}

static RoxStringBase *rox_add(const char *name, RoxStringBase *flag) {
    Rox *rox = rox_get_or_create();
    rox_core_add_flag(rox->core, flag, name);
    return flag;
}

ROX_API RoxStringBase *rox_add_flag(const char *name, bool default_value) {
    assert(name);
    return rox_add(name, variant_create_flag_with_default(default_value));
}

ROX_API RoxStringBase *rox_add_string(const char *name, const char *default_value) {
    return rox_add_string_with_options(name, default_value, NULL);
}

ROX_API RoxStringBase *rox_add_string_with_options(const char *name, const char *default_value, RoxList *options) {
    assert(name);
    return rox_add(name, variant_create_string(default_value, options));
}

ROX_API RoxStringBase *rox_add_int(const char *name, int default_value) {
    return rox_add_int_with_options(name, default_value, NULL);
}

ROX_API RoxStringBase *rox_add_int_with_options(const char *name, int default_value, RoxList *options) {
    assert(name);
    return rox_add(name, variant_create_int(default_value, options));
}

ROX_API RoxStringBase *rox_add_double(const char *name, double default_value) {
    return rox_add_double_with_options(name, default_value, NULL);
}

ROX_API RoxStringBase *rox_add_double_with_options(const char *name, double default_value, RoxList *options) {
    assert(name);
    return rox_add(name, variant_create_double(default_value, options));
}

ROX_API char *rox_get_string(RoxStringBase *variant) {
    assert(variant);
    EvaluationContext *eval_context = eval_context_create(variant, NULL);
    char *result = variant_get_string(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API char *rox_get_string_ctx(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    char *result = variant_get_string(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API int rox_get_int(RoxStringBase *variant) {
    assert(variant);
    EvaluationContext *eval_context = eval_context_create(variant, NULL);
    int result = variant_get_int(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API int rox_get_int_ctx(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    int result = variant_get_int(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API double rox_get_double(RoxStringBase *variant) {
    assert(variant);
    EvaluationContext *eval_context = eval_context_create(variant, NULL);
    double result = variant_get_double(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API double rox_get_double_ctx(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    double result = variant_get_double(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API bool rox_is_enabled(RoxStringBase *variant) {
    assert(variant);
    EvaluationContext *eval_context = eval_context_create(variant, NULL);
    bool result = variant_get_bool(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API bool rox_is_enabled_ctx(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    EvaluationContext *eval_context = eval_context_create(variant, context);
    bool result = variant_get_bool(variant, NULL, eval_context);
    eval_context_free(eval_context);
    return result;
}

ROX_API void rox_enabled_do(RoxStringBase *variant, void *target, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (rox_is_enabled(variant)) {
        action(target);
    }
}

ROX_API void
rox_enabled_do_ctx(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action) {
    assert(variant);
    assert(action);
    assert(context);
    if (rox_is_enabled_ctx(variant, context)) {
        action(target);
    }
}

ROX_API void rox_disabled_do(RoxStringBase *variant, void *target, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (!rox_is_enabled(variant)) {
        action(target);
    }
}

ROX_API void rox_disabled_do_ctx(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action) {
    assert(variant);
    assert(action);
    assert(context);
    if (!rox_is_enabled_ctx(variant, context)) {
        action(target);
    }
}

static void add_custom_prop(const char *name, const CustomPropertyType *type, void *target,
                            rox_custom_property_value_generator generator) {
    assert(name);
    assert(type);
    assert(generator);
    Rox *rox = rox_get_or_create();
    rox_core_add_custom_property(rox->core, custom_property_create(name, type, target, generator));
}

static void add_custom_prop_value(const char *name, const CustomPropertyType *type, RoxDynamicValue *value) {
    assert(name);
    assert(value);
    Rox *rox = rox_get_or_create();
    rox_core_add_custom_property(rox->core, custom_property_create_using_value(name, type, value));
}

ROX_API void rox_set_custom_string_property(const char *name, const char *value) {
    assert(name);
    assert(value);
    add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, rox_dynamic_value_create_string_copy(value));
}

ROX_API void rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_STRING, target, generator);
}

ROX_API void rox_set_custom_boolean_property(const char *name, bool value) {
    assert(name);
    add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, rox_dynamic_value_create_boolean(value));
}

ROX_API void rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_BOOL, target, generator);
}

ROX_API void rox_set_custom_double_property(const char *name, double value) {
    assert(name);
    add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, rox_dynamic_value_create_double(value));
}

ROX_API void rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_DOUBLE, target, generator);
}

ROX_API void rox_set_custom_integer_property(const char *name, int value) {
    assert(name);
    add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, rox_dynamic_value_create_int(value));
}

ROX_API void rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_INT, target, generator);
}

ROX_API void rox_set_custom_semver_property(const char *name, const char *value) {
    assert(name);
    add_custom_prop_value(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, rox_dynamic_value_create_string_copy(value));
}

ROX_API void rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator) {
    assert(name);
    assert(generator);
    add_custom_prop(name, &ROX_CUSTOM_PROPERTY_TYPE_SEMVER, target, generator);
}

static bool is_valid_state_for_shutdown() {
    if (!rox_global) {
        ROX_WARN("Cannot shut down; rox_setup wasn't called");
        return false;
    }
    if ((rox_global->state == RoxInitialized) || is_error_state(rox_global->state)) {
        return true;
    }
    ROX_WARN("Cannot shut down; current state is %d", rox_global->state);
    return false;
}

ROX_API void rox_shutdown() {
    pthread_mutex_lock(&startup_shutdown_lock);
    if (!is_valid_state_for_shutdown()) {
        pthread_mutex_unlock(&startup_shutdown_lock);
        return;
    }
    reset_state();
    pthread_mutex_unlock(&startup_shutdown_lock);
}

ROX_API RoxDynamicApi *rox_dynamic_api() {
    if (!check_setup_called()) {
        return NULL;
    }
    return rox_core_create_dynamic_api(rox_global->core, rox_global->entities_provider);
}
