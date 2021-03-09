#pragma once

#include <stddef.h>
#include <rox/defs.h>
#include <rox/flags.h>

typedef struct FlagOverrides FlagOverrides;

/**
 * @return Not <code>NULL</code>.
 */
ROX_API FlagOverrides *rox_get_overrides();

/**
 * @param overrides Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @return Whether flag with name <code>name</code> has overridden value.
 */
ROX_API bool rox_has_override(FlagOverrides *overrides, const char *name);

/**
 * @param overrides Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_override(FlagOverrides *overrides, const char *name, const char *value);

/**
 * Returns the overridden value for the given flag name.
 * The returned value must NOT be freed by the caller.
 *
 * @param overrides Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_API const char *rox_get_override(FlagOverrides *overrides, const char *name);

/**
 * @param overrides Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_API void rox_clear_override(FlagOverrides *overrides, const char *name);

/**
 * @param overrides Not <code>NULL</code>.
 */
ROX_API void rox_clear_overrides(FlagOverrides *overrides);

/**
 * Retrieves the current flag value without freeze, and without invoking impression.
 * The returned value, if not <code>NULL</code>, must be freed by the caller.
 *
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_API char *rox_peek_current_value(RoxStringBase *variant);

/**
 * Retrieves the original value with no overrides, no freeze, and without invoking impression.
 * The returned value, if not <code>NULL</code>, must be freed by the caller.
 *
 * @param variant Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_API char *rox_peek_original_value(RoxStringBase *variant);
