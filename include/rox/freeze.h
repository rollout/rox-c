#pragma once

#include <rox/macros.h>

typedef enum RoxFreeze {
    RoxFreezeUntilLaunch = 1,
    RoxFreezeUntilForeground = 2,
    RoxFreezeNone = 3
} RoxFreeze;

// TODO: rox_add_flag_with_freeze
// TODO: rox_add_string_with_freeze
// TODO: rox_add_string_with_freeze
// TODO: rox_add_string_with_freeze_and_options
// TODO: rox_add_int_with_freeze
// TODO: rox_add_int_with_freeze
// TODO: rox_add_int_with_freeze_and_options
// TODO: rox_add_double_with_freeze
// TODO: rox_add_double_with_freeze_and_options
// TODO: rox_freeze_flag
// TODO: rox_unfreeze_flag

/**
 * Unfreeze all flags.
 */
ROX_API void rox_unfreeze();

/**
 * Unfreeze all flags in namespace <code>ns</code>.
 */
ROX_API void rox_unfreeze_ns(const char *ns);
