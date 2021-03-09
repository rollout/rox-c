#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "rox/freeze.h"
#include "core/entities.h"
#include "core/configuration.h"
#include "core/options.h"
#include "core.h"
#include "freeze.h"
#include "util.h"

typedef struct FreezeContext {
    RoxFreeze freeze;
    bool frozen;
    char *frozen_value;
    bool use_default_freeze;
    variant_eval_func original_eval_func;
} FreezeContext;

static const char *KEY = "freeze";

static RoxFreeze default_freeze = RoxFreezeNone;
static FlagRepository *flag_repository = NULL;
static ConfigurationFetchedInvoker *conf_fetched_invoker = NULL;
static void *flag_added_callback_handle = NULL;
static void *configuration_fetched_callback_handle = NULL;

static void flag_freeze_added_callback(void *target, RoxStringBase *variant) {
    rox_freeze_flag(variant, default_freeze);
}

static void unfreeze_configuration_fetched_handler(void *target, RoxConfigurationFetchedArgs *args) {
    // Unfreeze on the very first conf fetched event, regardless on the fetch status
    configuration_fetched_invoker_unregister_handler(target, configuration_fetched_callback_handle);
    configuration_fetched_callback_handle = NULL;
    rox_unfreeze();
}

ROX_API void rox_options_set_default_freeze(RoxOptions *options, RoxFreeze freeze) {
    assert(options);
    rox_options_set_extra(options, KEY, mem_copy_int(freeze), free);
}

ROX_INTERNAL void rox_freeze_init(RoxCore *core, RoxOptions *options) {
    assert(core);

    flag_repository = rox_core_get_flag_repository(core);
    RoxFreeze *freeze = rox_options_get_extra(options, KEY);
    if (freeze) {
        default_freeze = *freeze;
    }

    flag_added_callback_handle = flag_repository_add_flag_added_callback(
            flag_repository, NULL,
            flag_freeze_added_callback);

    RoxMap *all_flags = flag_repository_get_all_flags(flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxStringBase *flag = (RoxStringBase *) value;
        rox_freeze_flag(flag, default_freeze);
    })

    conf_fetched_invoker = rox_core_get_configuration_fetched_invoker(core);
    configuration_fetched_callback_handle = configuration_fetched_invoker_register_handler(
            conf_fetched_invoker,
            conf_fetched_invoker,
            unfreeze_configuration_fetched_handler);
}

ROX_INTERNAL void rox_freeze_uninit() {

    if (flag_repository) {
        if (flag_added_callback_handle) {
            flag_repository_remove_flag_added_callback(
                    flag_repository,
                    flag_added_callback_handle);
            flag_added_callback_handle = NULL;
        }
        flag_repository = NULL;
    }

    if (conf_fetched_invoker) {
        if (configuration_fetched_callback_handle) {
            configuration_fetched_invoker_unregister_handler(
                    conf_fetched_invoker,
                    configuration_fetched_callback_handle);
            configuration_fetched_callback_handle = NULL;
        }
        conf_fetched_invoker = NULL;
    }

    default_freeze = RoxFreezeNone;
}

static void freeze_context_free(FreezeContext *context) {
    assert(context);
    if (context->frozen_value) {
        free(context->frozen_value);
    }
    free(context);
}

static RoxDynamicValue *freezable_flag_eval_func(
        RoxStringBase *variant,
        const char *default_value,
        EvaluationContext *eval_context,
        FlagValueConverter *converter) {

    FreezeContext *context = variant_get_data(variant, KEY);
    if (!context) {
        variant_eval_func eval_func = variant_get_eval_func(variant);
        return eval_func(variant, default_value, eval_context, converter);
    }

    if ((eval_context && !eval_context_is_use_freeze(eval_context)) ||
        context->freeze == RoxFreezeNone) {
        return context->original_eval_func(variant, default_value, eval_context, converter);
    }

    if (!context->frozen) {
        context->frozen = true;
        RoxDynamicValue *value = context->original_eval_func(variant, default_value,
                                                             eval_context, converter);
        context->frozen_value = converter->to_string(value);
        rox_dynamic_value_free(value);
    }

    return converter->from_string(context->frozen_value);
}

static FreezeContext *get_freeze_context(RoxStringBase *flag) {
    assert(flag);
    FreezeContext *context = variant_get_data(flag, KEY);
    if (!context) {
        context = calloc(1, sizeof(FreezeContext));
        context->frozen = false;
        context->freeze = RoxFreezeNone;
        context->use_default_freeze = true;
        context->original_eval_func = variant_get_eval_func(flag);
        VariantConfig config = {freezable_flag_eval_func};
        variant_set_config(flag, &config);
        variant_add_data(flag, KEY, context, (variant_free_data_func) freeze_context_free);
    }
    return context;
}

static RoxStringBase *add_freeze(RoxStringBase *flag, RoxFreeze freeze) {
    FreezeContext *context = get_freeze_context(flag);
    context->freeze = freeze;
    context->use_default_freeze = false;
    return flag;
}

ROX_API RoxStringBase *rox_add_flag_with_freeze(const char *name, bool default_value, RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_flag(name, default_value), freeze);
}

ROX_API RoxStringBase *rox_add_string_with_freeze(
        const char *name,
        const char *default_value,
        RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_string(name, default_value), freeze);
}

ROX_API RoxStringBase *rox_add_string_with_freeze_and_options(
        const char *name,
        const char *default_value,
        RoxList *options,
        RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_string_with_options(name, default_value, options), freeze);
}

ROX_API RoxStringBase *rox_add_int_with_freeze(const char *name, int default_value, RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_int(name, default_value), freeze);
}

ROX_API RoxStringBase *rox_add_int_with_freeze_and_options(
        const char *name,
        int default_value,
        RoxList *options,
        RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_int_with_options(name, default_value, options), freeze);
}

ROX_API RoxStringBase *rox_add_double_with_freeze(const char *name, double default_value, RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_double(name, default_value), freeze);
}

ROX_API RoxStringBase *rox_add_double_with_freeze_and_options(
        const char *name,
        double default_value,
        RoxList *options,
        RoxFreeze freeze) {
    assert(name);
    return add_freeze(rox_add_double_with_options(name, default_value, options), freeze);
}

ROX_API void rox_freeze_flag(RoxStringBase *flag, RoxFreeze freeze) {
    assert(flag);
    FreezeContext *context = get_freeze_context(flag);
    if (!context->use_default_freeze) {
        return;
    }
    context->freeze = freeze;
}

ROX_API void rox_unfreeze_flag(RoxStringBase *flag, RoxFreeze freeze) {
    assert(flag);
    FreezeContext *context = variant_get_data(flag, KEY);
    if (context && freeze <= context->freeze) {
        context->frozen = false;
        if (context->frozen_value) {
            free(context->frozen_value);
            context->frozen_value = NULL;
        }
    }
}

ROX_API void rox_unfreeze() {
    if (!flag_repository) {
        return;
    }
    RoxMap *all_flags = flag_repository_get_all_flags(flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxStringBase *flag = (RoxStringBase *) value;
        rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
    })
}

static bool is_flag_in_namespace(RoxStringBase *flag, const char *ns) {
    const char *name = variant_get_name(flag);
    return str_starts_with(name, ns) && name[strlen(ns)] == '.';
}

ROX_API void rox_unfreeze_ns(const char *ns) {
    assert(ns);
    if (!flag_repository) {
        return;
    }
    RoxMap *all_flags = flag_repository_get_all_flags(flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxStringBase *flag = (RoxStringBase *) value;
        if (is_flag_in_namespace(flag, ns)) {
            rox_unfreeze_flag(flag, RoxFreezeUntilLaunch);
        }
    })
}

ROX_INTERNAL bool rox_flag_is_frozen(RoxStringBase *flag) {
    FreezeContext *context = variant_get_data(flag, KEY);
    return context ? context->frozen : false;
}

ROX_INTERNAL RoxFreeze rox_flag_get_freeze(RoxStringBase *flag) {
    FreezeContext *context = variant_get_data(flag, KEY);
    return context ? context->freeze : RoxFreezeNone;
}
