#pragma once

#include "rox/defs.h"
#include "rox/options.h"

typedef void (*rox_options_free_extra_func)(void *data);

/**
 * Sets extra payload data for options.
 *
 * Can be used for example to extend options with feature flag settings,
 * storage settings and so on. FOR INTERNAL USE ONLY.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 */
ROX_INTERNAL void rox_options_set_extra(
        RoxOptions *options,
        const char *key,
        void *data,
        rox_options_free_extra_func free_func);

/**
 * Gets extra data previously added via <code>rox_options_set_extra</code>.
 * FOR INTERNAL USE ONLY.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL void *rox_options_get_extra(RoxOptions *options, const char *key);
