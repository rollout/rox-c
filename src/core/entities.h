#pragma once

#include "rollout.h"
#include "core/impression.h"
#include "roxx/parser.h"

//
// Variant
//

/**
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 *
 * @param default_value May be <code>NULL</code>. Value is copied internally so the caller holds ownership.
 * @param options List of strings. May be <code>NULL</code>. If passed, ownership is delegated to variant.
 * @return Not <code>NULL</code>.
 */
RoxVariant *ROX_INTERNAL variant_create(const char *default_value, List *options);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
const char *ROX_INTERNAL variant_get_name(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
const char *ROX_INTERNAL variant_get_default_value(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
const char *ROX_INTERNAL variant_get_condition(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ExperimentModel *ROX_INTERNAL variant_get_experiment(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
Parser *ROX_INTERNAL variant_get_parser(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ImpressionInvoker *ROX_INTERNAL variant_get_impression_invoker(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
bool ROX_INTERNAL variant_is_flag(RoxVariant *variant);

/**
 * @param variant Not <code>NULL</code>.
 */
List *ROX_INTERNAL variant_get_options(RoxVariant *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if the value is not defined.
 */
char *ROX_INTERNAL variant_get_value_or_default(RoxVariant *variant, RoxContext *context);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>NULL</code>, if the value is not defined.
 */
char *ROX_INTERNAL variant_get_value_or_null(RoxVariant *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
bool ROX_INTERNAL flag_is_enabled(RoxVariant *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @return <code>true</code> or <code>false</code> or <code>NULL</code>.
 */
const bool *ROX_INTERNAL flag_is_enabled_or_null(RoxVariant *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @param action Not <code>NULL</code>.
 */
void ROX_INTERNAL flag_enabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @param action Not <code>NULL</code>.
 */
void ROX_INTERNAL flag_disabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action);

/**
 * Ownership on <code>parser</code>, <code>experiment</code>,
 * and <code>impression_invoker</code>, if passed, is hold by the caller.
 *
 * @param variant Not <code>NULL</code>.
 * @param parser May be <code>NULL</code>. The ownership is NOT delegated.
 * @param experiment May be <code>NULL</code>. The ownership is NOT delegated.
 * @param impression_invoker May be <code>NULL</code>. The ownership is NOT delegated.
 */
void ROX_INTERNAL variant_set_for_evaluation(
        RoxVariant *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker);

/**
 * Ownership on <code>context</code> is hold by the caller.
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
void ROX_INTERNAL variant_set_context(RoxVariant *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. The given string is copied internally.
 */
void ROX_INTERNAL variant_set_name(RoxVariant *variant, const char *name);

/**
 * @param variant Not <code>NULL</code>.
 * @param condition Not <code>NULL</code>. The given string is copied internally.
 */
void ROX_INTERNAL variant_set_condition(RoxVariant *variant, const char *condition);

/**
 * @param variant Not <code>NULL</code>.
 */
void ROX_INTERNAL variant_free(RoxVariant *variant);

//
// Flag
//

extern const char *ROX_INTERNAL FLAG_TRUE_VALUE;
extern const char *ROX_INTERNAL FLAG_FALSE_VALUE;

/**
 * Creates flag with <code>false</code> being a default value.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @return Not <code>NULL</code>.
 */
RoxVariant *ROX_INTERNAL variant_create_flag();

/**
 * Creates flag with the given <code>default_value</code>.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @return Not <code>NULL</code>.
 */
RoxVariant *ROX_INTERNAL variant_create_flag_with_default(bool default_value);

//
// FlagSetter
//

typedef struct ROX_INTERNAL FlagSetter FlagSetter;

typedef struct ROX_INTERNAL FlagRepository FlagRepository;

typedef struct ROX_INTERNAL ExperimentRepository ExperimentRepository;

/**
 * The returned object must be destroyed after use by calling <code>flag_setter_free</code>.
 *
 * @param flag_repository Not <code>NULL</code>. The callers holds the ownership.
 * @param parser Not <code>NULL</code>. The callers holds the ownership.
 * @param experiment_repository Not <code>NULL</code>. The callers holds the ownership.
 * @param impression_invoker May be <code>NULL</code>. The callers holds the ownership.
 * @return Not <code>NULL</code>.
 */
FlagSetter *ROX_INTERNAL flag_setter_create(
        FlagRepository *flag_repository,
        Parser *parser,
        ExperimentRepository *experiment_repository,
        ImpressionInvoker *impression_invoker);

/**
 * @param flag_setter Not <code>NULL</code>.
 */
void ROX_INTERNAL flag_setter_set_experiments(FlagSetter *flag_setter);

/**
 * @param flag_setter Not <code>NULL</code>.
 */
void ROX_INTERNAL flag_setter_free(FlagSetter *flag_setter);

//
// EntitiesProvider
//

typedef struct ROX_INTERNAL EntitiesProvider EntitiesProvider;

/**
 * The returned object must be destroyed after use by calling <code>entities_provider_free()</code>.
 * @return Not <code>NULL</code>.
 */
EntitiesProvider *ROX_INTERNAL entities_provider_create();

/**
 * @param  provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
RoxVariant *ROX_INTERNAL entities_provider_create_flag(EntitiesProvider *provider, bool default_value);

/**
 * @param provider Not <code>NULL</code>.
 * @param defaultValue  May be <code>NULL</code>.
 * @param options List of strings. May be <code>NULL</code>. Ownership of this list is delegated to the <code>provider</code>.
 * @return Not <code>NULL</code>.
 */
RoxVariant *ROX_INTERNAL entities_provider_create_variant(
        EntitiesProvider *provider,
        const char *defaultValue,
        List *options);

/**
 * @param entities_provider Not <code>NULL</code>.
 */
void ROX_INTERNAL entities_provider_free(EntitiesProvider *provider);
