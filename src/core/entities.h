#pragma once

#include "roxapi.h"
#include "roxx/parser.h"
#include "configuration/models.h"
#include "impression.h"
#include "repositories.h"

//
// Variant
//

typedef struct ROX_INTERNAL Variant Variant;

/**
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @param default_value Not <code>NULL</code>.
 * @param options List of strings. May be <code>NULL</code>. If passed, ownership is delegated to variant.
 * @return Not <code>NULL</code>.
 */
Variant *ROX_INTERNAL variant_create(const char *default_value, List *options);

/**
 * @param variant Not <code>NULL</code>.
 * @param parser May be <code>NULL</code>.
 * @param experiment May be <code>NULL</code>.
 * @param impressionInvoker May be <code>NULL</code>.
 */
void ROX_INTERNAL variant_set_for_evaluation(
        Variant *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impressionInvoker);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
void ROX_INTERNAL variant_set_context(Variant *variant, Context *context);

/**
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
char *ROX_INTERNAL variant_get_name(Variant *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. The given string is copied internally.
 */
void ROX_INTERNAL variant_set_name(Variant *variant, const char *name);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>default_value</code> passed to <code>create_variant()</code>, if the value is not defined.
 */
char *ROX_INTERNAL variant_get_value_or_default(Variant *variant, Context *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>
 * @return Current value or <code>NULL</code>, if the value is not defined.
 */
char *ROX_INTERNAL variant_get_value_or_null(Variant *variant, Context *context);

/**
 * @param variant Not <code>NULL</code>.
 */
void ROX_INTERNAL variant_free(Variant *variant);

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
Variant *ROX_INTERNAL variant_create_flag();

/**
 * Creates flag with the given <code>default_value</code>.
 * The returned object must be freed after use by calling <code>variant_free()</code>.
 * @return Not <code>NULL</code>.
 */
Variant *ROX_INTERNAL variant_create_flag_with_default(bool default_value);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
bool ROX_INTERNAL flag_is_enabled(Variant *variant, Context *context);

/**
 * @param variant Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @return <code>true</code> or <code>false</code> or <code>NULL</code>.
 */
bool *ROX_INTERNAL flag_is_enabled_or_null(Variant *variant, Context *context);

typedef ROX_INTERNAL void (*flag_action)();

void ROX_INTERNAL flag_enabled_do(Variant *variant, Context *context, flag_action action);

void ROX_INTERNAL flag_disabled_do(Variant *variant, Context *context, flag_action action);

//
// FlagSetter
//

typedef struct ROX_INTERNAL FlagSetter FlagSetter;

/**
 * The returned object myst be destroyed after use by calling <code>flag_setter_free</code>.
 * @param flag_repository Not <code>NULL</code>.
 * @param experiment_repository Not <code>NULL</code>.
 * @param impression_invoker May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
FlagSetter *ROX_INTERNAL flag_setter_create(
        FlagRepository *flag_repository,
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
EntitiesProvider *entities_provider_create();

/**
 * @param  provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
Variant *entities_provider_create_flag(EntitiesProvider *provider);

/**
 * @param provider  Not <code>NULL</code>.
 * @param defaultValue  Not <code>NULL</code>.
 * @param options List of strings. Not <code>NULL</code>. Ownership of this list is delegated to provider.
 * @return Not <code>NULL</code>.
 */
Variant *entities_provider_create_variant(
        EntitiesProvider *provider,
        const char *defaultValue,
        List *options);

/**
 * @param entities_provider Not <code>NULL</code>.
 */
void entities_provider_free(EntitiesProvider *provider);
