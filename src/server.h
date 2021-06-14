#pragma once

#include "rox/defs.h"

/**
 * FOR TESTING ONLY.
 * @param config May be <code>NULL</code>. In case of <code>NULL</code>, the request config will be set to default one.
 */
ROX_INTERNAL void rox_set_default_request_config(RequestConfig *config);
