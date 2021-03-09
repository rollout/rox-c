#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rox/storage.h"
#include "rox/overrides.h"
#include "rox/freeze.h"
#include "overrides.h"
#include "core/logging.h"
#include "storage.h"

struct FlagOverrides {
    RoxMap *values;
    StorageEntry *storage_entry;
};

static const char *FLAG_DATA_KEY = "overrides";
static const char *STORAGE_ENTRY_KEY = "overrides";

static Storage *storage = NULL;
static FlagOverrides *global_overrides = NULL;
static FlagRepository *flag_repository = NULL;
static void *flag_added_callback_handle = NULL;

static FlagOverrides *flag_overrides_create(StorageEntry *entry) {
    FlagOverrides *overrides = calloc(1, sizeof(FlagOverrides));
    overrides->storage_entry = entry;
    if (entry) {
        overrides->values = storage_read_string_key_value_map(entry);
    }
    if (!overrides->values) {
        overrides->values = ROX_EMPTY_MAP;
    }
    return overrides;
}

static void flag_overrides_free(FlagOverrides *overrides) {
    assert(overrides);
    rox_map_free_with_keys_and_values_cb(overrides->values, free, free);
    free(overrides);
}

typedef struct OverrideFlagData {
    FlagOverrides *overrides;
    variant_eval_func original_eval_func;
} OverrideFlagData;

static OverrideFlagData *override_flag_data_create(RoxStringBase *variant, FlagOverrides *overrides) {
    assert(variant);
    assert(overrides);
    OverrideFlagData *data = calloc(1, sizeof(OverrideFlagData));
    data->overrides = overrides;
    data->original_eval_func = variant_get_eval_func(variant);
    return data;
}

static void override_flag_data_free(OverrideFlagData *data) {
    assert(data);
    free(data);
}

static RoxDynamicValue *override_flag_eval_func(
        RoxStringBase *variant,
        const char *default_value,
        EvaluationContext *eval_context,
        FlagValueConverter *converter) {

    OverrideFlagData *override = variant_get_data(variant, FLAG_DATA_KEY);
    if (!override ||
        !override->overrides ||
        (eval_context && !eval_context_is_use_overrides(eval_context))) {
        return override->original_eval_func(
                variant, default_value,
                eval_context, converter);
    }

    void *value;
    const char *key = variant_get_name(variant);
    if (rox_map_get(override->overrides->values, (void *) key, &value)) {
        return converter->from_string(value);
    }

    return override->original_eval_func(
            variant, default_value,
            eval_context, converter);
}

static void init_flag_overrides(RoxStringBase *variant, FlagOverrides *overrides) {
    assert(variant);
    OverrideFlagData *data = override_flag_data_create(variant, overrides);
    VariantConfig config = {override_flag_eval_func};
    variant_add_data(variant, FLAG_DATA_KEY, data, (variant_free_data_func) override_flag_data_free);
    variant_set_config(variant, &config);
}

static void override_flag_added_callback(void *target, RoxStringBase *variant) {
    assert(variant);
    assert(target);
    FlagOverrides *overrides = target;
    init_flag_overrides(variant, overrides);
}

ROX_INTERNAL void rox_overrides_init(RoxCore *core, RoxOptions *options) {
    assert(core);
    assert(options);

    rox_overrides_uninit(); // free all existing data, if any

    StorageConfig *storage_config = get_storage_config_from_options(options);
    storage = storage_create(storage_config);

    StorageEntry *storage_entry = storage_get_entry(storage, STORAGE_ENTRY_KEY);
    FlagOverrides *overrides = flag_overrides_create(storage_entry);
    global_overrides = overrides;

    flag_repository = rox_core_get_flag_repository(core);
    flag_added_callback_handle = flag_repository_add_flag_added_callback(
            flag_repository, overrides,
            override_flag_added_callback);

    RoxMap *all_flags = flag_repository_get_all_flags(flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxStringBase *flag = (RoxStringBase *) value;
        init_flag_overrides(flag, overrides);
    })
}

ROX_INTERNAL void rox_overrides_uninit() {

    if (flag_repository) {
        if (flag_added_callback_handle) {
            flag_repository_remove_flag_added_callback(flag_repository, flag_added_callback_handle);
            flag_added_callback_handle = NULL;
        }
        flag_repository = NULL;
    }

    if (global_overrides) {
        flag_overrides_free(global_overrides);
        global_overrides = NULL;
    }

    if (storage) {
        storage_free(storage);
        storage = NULL;
    }
}

ROX_API FlagOverrides *rox_get_overrides() {
    if (!global_overrides) {
        ROX_WARN("rox_get_overrides is called before rox_setup");
        global_overrides = flag_overrides_create(NULL);
    }
    return global_overrides;
}

ROX_API bool rox_has_override(FlagOverrides *overrides, const char *name) {
    assert(overrides);
    assert(name);
    return rox_map_contains_key(overrides->values, (void *) name);
}

ROX_API void rox_set_override(FlagOverrides *overrides, const char *name, const char *value) {
    assert(overrides);
    assert(name);
    assert(value);
    if (rox_map_contains_key(overrides->values, (void *) name)) {
        rox_map_remove_key_value_cb(overrides->values, (void *) name, free, free);
    }
    rox_map_add(overrides->values, mem_copy_str(name), mem_copy_str(value));
}

ROX_API const char *rox_get_override(FlagOverrides *overrides, const char *name) {
    assert(overrides);
    assert(name);
    void *data;
    if (rox_map_get(overrides->values, (void *) name, &data)) {
        return data;
    }
    return NULL;
}

ROX_API void rox_clear_override(FlagOverrides *overrides, const char *name) {
    assert(overrides);
    if (rox_map_contains_key(overrides->values, (void *) name)) {
        rox_map_remove_key_value_cb(overrides->values, (void *) name, free, free);
        storage_write_string_key_value_map(overrides->storage_entry, overrides->values);
        if (flag_repository) {
            RoxStringBase *flag = flag_repository_get_flag(flag_repository, name);
            if (flag) {
                rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
            }
        }
    }
}

ROX_API void rox_clear_overrides(FlagOverrides *overrides) {
    assert(overrides != NULL);
    storage_delete_entry(overrides->storage_entry);
    RoxMap *old_values = overrides->values;
    overrides->values = ROX_EMPTY_MAP;
    rox_map_free_with_keys_and_values_cb(old_values, free, free);
    rox_unfreeze();
}

ROX_API char *rox_peek_current_value(RoxStringBase *variant) {
    assert(variant);
    EvalContextConfig config = {variant, NULL, false, false, true};
    EvaluationContext *eval_context = eval_context_create_custom(&config);
    char *value = variant_get_value_as_string(variant, eval_context);
    eval_context_free(eval_context);
    return value;
}

ROX_API char *rox_peek_original_value(RoxStringBase *variant) {
    assert(variant);
    EvalContextConfig config = {variant, NULL, false, false, false};
    EvaluationContext *eval_context = eval_context_create_custom(&config);
    char *value = variant_get_value_as_string(variant, eval_context);
    eval_context_free(eval_context);
    return value;
}
