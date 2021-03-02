#pragma once

#include "rox/server.h"
#include "core/impression.h"
#include "roxx/parser.h"

//
// String
//

/**
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 *
 * @param default_value May be <code>NULL</code>. Value is copied internally so the caller holds ownership.
 * @param options List of strings. May be <code>NULL</code>. If passed, ownership is delegated to variant.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *variant_create_string(const char *default_value, RoxList *options);

/**
 * Creates integer flag with the given <code>default_value</code>.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 *
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *variant_create_int(int default_value, RoxList *options);

/**
 * Creates double flag with the given <code>default_value</code>.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 *
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *variant_create_double(double default_value, RoxList *options);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL const char *variant_get_name(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL const char *variant_get_default_value(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL const char *variant_get_condition(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL ExperimentModel *variant_get_experiment(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL Parser *variant_get_parser(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL ImpressionInvoker *variant_get_impression_invoker(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL bool variant_is_flag(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL bool variant_is_string(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL bool variant_is_int(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL bool variant_is_double(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL RoxList *variant_get_options(RoxStringBase *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if variant's value is not defined.
 */
ROX_INTERNAL char *variant_get_string_or_default(RoxStringBase *variant, RoxContext *context);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code>, if variant's value is not defined.
 */
ROX_INTERNAL char *variant_get_string(RoxStringBase *variant, RoxContext *context, const char *default_value);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_INTERNAL bool flag_is_enabled(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_INTERNAL bool flag_is_enabled_or(RoxStringBase *variant, RoxContext *context, bool default_value);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @param target May be <code>NULL</code>
 * @param action Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_enabled_do(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @param target May be <code>NULL</code>
 * @param action Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_disabled_do(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code> passed to <code>variant_create_xxx()</code>, if variant's value is not defined.
 */
ROX_INTERNAL int variant_get_int_or_default(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code>, if variant's value is not defined.
 */
ROX_INTERNAL int variant_get_int_or(RoxStringBase *variant, RoxContext *context, int default_value);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code> passed to <code>variant_create_xxx()</code>, if variant's value is not defined.
 */
ROX_INTERNAL double variant_get_double_or_default(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code>, if variant's value is not defined.
 */
ROX_INTERNAL double variant_get_double_or(RoxStringBase *variant, RoxContext *context, double default_value);

/**
 * Ownership on <code>parser</code>, <code>experiment</code>,
 * and <code>impression_invoker</code>, if passed, is hold by the caller.
 *
 * @param variant Not <code>NULL</code>.
 * @param parser May be <code>NULL</code>. The ownership is NOT delegated.
 * @param experiment May be <code>NULL</code>. The ownership is NOT delegated. If passed, value is copied internally.
 * @param impression_invoker May be <code>NULL</code>. The ownership is NOT delegated.
 */
ROX_INTERNAL void variant_set_for_evaluation(
        RoxStringBase *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker);

/**
 * Ownership on <code>context</code> is hold by the caller.
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_INTERNAL void variant_set_context(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. The given string is copied internally.
 */
ROX_INTERNAL void variant_set_name(RoxStringBase *variant, const char *name);

/**
 * @param variant Not <code>NULL</code>.
 * @param condition Not <code>NULL</code>. The given string is copied internally.
 */
ROX_INTERNAL void variant_set_condition(RoxStringBase *variant, const char *condition);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_INTERNAL void variant_free(RoxStringBase *variant);

//
// Flag
//

ROX_INTERNAL extern const char *FLAG_TRUE_VALUE;
ROX_INTERNAL extern const char *FLAG_FALSE_VALUE;

/**
 * Creates flag with <code>false</code> being a default value.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *variant_create_flag();

/**
 * Creates flag with the given <code>default_value</code>.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *variant_create_flag_with_default(bool default_value);

//
// FlagSetter
//

typedef struct FlagSetter FlagSetter;

typedef struct FlagRepository FlagRepository;

typedef struct ExperimentRepository ExperimentRepository;

/**
 * The returned object must be destroyed after use by calling <code>flag_setter_free</code>.
 *
 * @param flag_repository Not <code>NULL</code>. The callers holds the ownership.
 * @param parser Not <code>NULL</code>. The callers holds the ownership.
 * @param experiment_repository Not <code>NULL</code>. The callers holds the ownership.
 * @param impression_invoker May be <code>NULL</code>. The callers holds the ownership.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL FlagSetter *flag_setter_create(
        FlagRepository *flag_repository,
        Parser *parser,
        ExperimentRepository *experiment_repository,
        ImpressionInvoker *impression_invoker);

/**
 * @param flag_setter Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_setter_set_experiments(FlagSetter *flag_setter);

/**
 * @param flag_setter Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_setter_free(FlagSetter *flag_setter);

//
// EntitiesProvider
//

typedef struct EntitiesProvider EntitiesProvider;

/**
 * The returned object must be destroyed after use by calling <code>entities_provider_free()</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL EntitiesProvider *entities_provider_create();

/**
 * @param  provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *entities_provider_create_flag(EntitiesProvider *provider, bool default_value);

/**
 * @param provider Not <code>NULL</code>.
 * @param defaultValue  May be <code>NULL</code>.
 * @param options List of strings. May be <code>NULL</code>. Ownership of this list is delegated to the <code>provider</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *entities_provider_create_string(
        EntitiesProvider *provider,
        const char *defaultValue,
        RoxList *options);

/**
 * @param provider Not <code>NULL</code>.
 * @param defaultValue  May be <code>NULL</code>.
 * @param options List of ints. May be <code>NULL</code>. Ownership of this list is delegated to the <code>provider</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *entities_provider_create_int(
        EntitiesProvider *provider,
        int defaultValue,
        RoxList *options);

/**
 * @param provider Not <code>NULL</code>.
 * @param defaultValue  May be <code>NULL</code>.
 * @param options List of doubles. May be <code>NULL</code>. Ownership of this list is delegated to the <code>provider</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *entities_provider_create_double(
        EntitiesProvider *provider,
        double defaultValue,
        RoxList *options);

/**
 * @param entities_provider Not <code>NULL</code>.
 */
ROX_INTERNAL void entities_provider_free(EntitiesProvider *provider);
