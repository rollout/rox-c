#pragma once

#include <rox/macros.h>
#include <rox/context.h>
#include <rox/options.h>
#include <rox/errors.h>

/**
 * Must be called before any other <code>rox_xxx</code> calls.
 *
 * @param api_key Not <code>NULL</code>.
 * @param options May be <code>NULL</code>. If passed, the ownership is delegated to ROX.
 */
ROX_API RoxStateCode rox_setup(const char *api_key, RoxOptions *options);

/**
 * @param context May be <code>NULL</code>. The ownership is delegated to ROX.
 */
ROX_API void rox_set_context(RoxContext *context);

ROX_API void rox_fetch();

/**
 * Should be called to free all the rox memory. After this,
 * no <code>rox_xxx</code> method can be called anymore.
 */
ROX_API void rox_shutdown();
