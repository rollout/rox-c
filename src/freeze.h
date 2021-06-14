#pragma once

#include <stdbool.h>
#include "rox/defs.h"
#include "rox/freeze.h"
#include "core.h"
#include "core/options.h"

ROX_INTERNAL void rox_freeze_init(RoxCore *core, RoxOptions *options);

ROX_INTERNAL void rox_freeze_uninit();

ROX_INTERNAL bool rox_flag_is_frozen(RoxStringBase *flag);

ROX_INTERNAL RoxFreeze rox_flag_get_freeze(RoxStringBase *flag);
