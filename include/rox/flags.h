#pragma once

#include <rox/macros.h>
#include <rox/context.h>
#include <rox/collections.h>

/**
 * Note: the name RoxStringBase was taken for compatibility with other ROX SDKs.
 */
typedef struct RoxStringBase RoxStringBase;

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_flag(const char *name, bool default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string(const char *name, const char *default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string_with_options(const char *name, const char *default_value, RoxList *options);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int(const char *name, int default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int_with_options(const char *name, int default_value, RoxList *options);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double(const char *name, double default_value);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double_with_options(const char *name, double default_value, RoxList *options);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_string()</code>, if the value is not defined.
 */
ROX_API char *rox_get_string(RoxStringBase *variant);

/**
 * The returned value must be freed after use by the caller, if not <code>NULL</code>.
 *
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_string()</code>, if the value is not defined.
 */
ROX_API char *rox_get_string_ctx(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
ROX_API bool rox_flag_is_enabled_ctx(RoxStringBase *variant, RoxContext *context);

typedef void (*rox_flag_action)(void *target);

/**
 * @param variant Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do(RoxStringBase *variant, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_enabled_do_ctx(RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do(RoxStringBase *variant, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param action Not <code>NULL</code>.
 */
ROX_API void rox_flag_disabled_do_ctx(
        RoxStringBase *variant, RoxContext *context, void *target, rox_flag_action action);

/**
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_int()</code>, if the value is not defined.
 */
ROX_API int rox_get_int(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_int()</code>, if the value is not defined.
 */
ROX_API int rox_get_int_ctx(RoxStringBase *variant, RoxContext *context);

/**
 * @param variant Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_double()</code>, if the value is not defined.
 */
ROX_API double rox_get_double(RoxStringBase *variant);

/**
 * @param variant Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 * @return Current value or <code>default_value</code> passed to <code>rox_add_double()</code>, if the value is not defined.
 */
ROX_API double rox_get_double_ctx(RoxStringBase *variant, RoxContext *context);
