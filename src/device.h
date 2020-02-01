#pragma once

#include "roxapi.h"

/**
 * The returned string, must <em>not</em> be freed by the caller.
 *
 * @return Can be <code>NULL</code>.
 */
const char *ROX_INTERNAL rox_globally_unique_device_id();