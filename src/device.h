#pragma once

#include "rollout.h"

/**
 * The returned string, must <em>not</em> be freed by the caller.
 *
 * @return Can be <code>NULL</code>.
 */
ROX_INTERNAL const char *rox_globally_unique_device_id();