#pragma once

#include "rox/defs.h"
#include "rox/options.h"

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
ROX_INTERNAL void rox_options_set_extra(RoxOptions *options, const char *key, void *data);

/**
 * Gets extra data previously added via <code>rox_options_set_extra</code>.
 * FOR INTERNAL USE ONLY.
 *
 * @param options Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 */
ROX_INTERNAL void *rox_options_get_extra(RoxOptions *options, const char *key);
