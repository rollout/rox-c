#pragma once

#include <rox/macros.h>

typedef struct RoxDynamicApi RoxDynamicApi;

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_API bool rox_dynamic_api_is_enabled(
        RoxDynamicApi *api,
        const char *name,
        bool default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_API bool rox_dynamic_api_is_enabled_ctx(
        RoxDynamicApi *api,
        const char *name,
        bool default_value,
        RoxContext *context);

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API char *rox_dynamic_api_get_string(
        RoxDynamicApi *api,
        const char *name,
        char *default_value);

/**
 * The returned value, if not <code>NULL</code>, must be freed after use by the caller.
 *
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API char *rox_dynamic_api_get_string_ctx(
        RoxDynamicApi *api,
        const char *name,
        char *default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 */
ROX_API int rox_dynamic_api_get_int(
        RoxDynamicApi *api,
        const char *name,
        int default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API int rox_dynamic_api_get_int_ctx(
        RoxDynamicApi *api,
        const char *name,
        int default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 */
ROX_API double rox_dynamic_api_get_double(
        RoxDynamicApi *api,
        const char *name,
        double default_value);

/**
 * @param api Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @param default_value Default flag value.
 * @param context May be <code>NULL</code>.
 * @param options May be <code>NULL</code>.
 */
ROX_API double rox_dynamic_api_get_double_ctx(
        RoxDynamicApi *api,
        const char *name,
        double default_value,
        RoxList *options,
        RoxContext *context);

/**
 * @param api Not <code>NULL</code>.
 */
ROX_API void rox_dynamic_api_free(RoxDynamicApi *api);

/**
 * Note the returned pointer must be freed after use by calling <code>rox_dynamic_api_free</code>.
 *
 * @return Not <code>NULL</code>.
 */
ROX_API RoxDynamicApi *rox_dynamic_api();