#pragma once

#include "core/impression.h"
#include "core/eval.h"
#include "eval/parser.h"

typedef void *(*variant_free_data_func)(void *data);

typedef struct VariantConfig {
    variant_eval_func eval_func;
} VariantConfig;

/**
 * @param variant Not <code>NULL</code>.
 * @param config Not <code>NULL</code>.
 */
ROX_INTERNAL void variant_set_config(RoxStringBase *variant, VariantConfig *config);

/**
 * @param variant Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @param free_data_func May be <code>NULL</code>.
 */
ROX_INTERNAL void variant_add_data(
        RoxStringBase *variant,
        const char *key,
        void *data,
        variant_free_data_func free_data_func);

/**
 * @param variant Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *variant_get_data(RoxStringBase *variant, const char *key);

/**
 * @param variant Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL variant_eval_func variant_get_eval_func(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>. Variant's default value will be used in this case.
 * @param eval_context May be <code>NULL</code>.
 * @param converter Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL RoxDynamicValue *variant_get_value(
        RoxStringBase *variant,
        const char *default_value,
        EvaluationContext *eval_context,
        FlagValueConverter *converter);

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
 * @param default_value May be <code>NULL</code>. Variant's default value will be used in this case.
 * @param eval_context May be <code>NULL</code>.
 * @return Current value or <code>default_value</code>, if variant's value is not defined.
 */
ROX_INTERNAL char *
variant_get_string(RoxStringBase *variant, const char *default_value, EvaluationContext *eval_context);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>. Variant's default value will be used in this case.
 * @param eval_context May be <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>variant_create_xxx()</code>, if variant's value is not defined.
 */
ROX_INTERNAL int variant_get_int(RoxStringBase *variant, const char *default_value, EvaluationContext *eval_context);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>. Variant's default value will be used in this case.
 * @param eval_context May be <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>variant_create_xxx()</code>, if variant's value is not defined.
 */
ROX_INTERNAL double
variant_get_double(RoxStringBase *variant, const char *default_value, EvaluationContext *eval_context);

/**
 * @param variant Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>. Variant's default value will be used in this case.
 * @param eval_context May be <code>NULL</code>.
 */
ROX_INTERNAL bool variant_get_bool(RoxStringBase *variant, const char *default_value, EvaluationContext *eval_context);

/**
 * Returns the string representation of the value, using the value's type defined in
 * its constructor. It first evaluates to the type-specific value, and then converted
 * to string using the type converted. This is needed for flag overrides mostly.
 *
 * The returned value must be freed by the caller.
 *
 * @param variant Not <code>NULL</code>.
 * @param eval_context May be <code>NULL</code>.
 * @return May be <code>NULL</code> if the flag value is <code>NULL</code>.
 */
ROX_INTERNAL char *variant_get_value_as_string(RoxStringBase *variant, EvaluationContext *eval_context);

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
