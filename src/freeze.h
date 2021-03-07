#pragma once

#include <stdbool.h>
#include "rox/defs.h"

ROX_INTERNAL bool rox_flag_is_frozen(RoxStringBase *flag);

ROX_INTERNAL RoxFreeze rox_flag_get_freeze(RoxStringBase *flag);
