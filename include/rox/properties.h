#pragma once

#include <rox/defs.h>
#include <rox/values.h>
#include <time.h>

typedef RoxDynamicValue *(*rox_custom_property_value_generator)(void *target, RoxContext *context);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_string_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_string_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_boolean_property(const char *name, bool value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_boolean_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_double_property(const char *name, double value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_double_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_integer_property(const char *name, int value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_integer_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_semver_property(const char *name, const char *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_semver_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param value Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_set_custom_datetime_property(const char *name, const struct tm *value);

/**
 * @param name Not <code>NULL</code>. Value is copied internally.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 */
ROX_API void rox_set_custom_computed_datetime_property(
        const char *name,
        void *target,
        rox_custom_property_value_generator generator);