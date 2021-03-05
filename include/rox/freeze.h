#pragma once

#include <stdbool.h>
#include <rox/defs.h>
#include <rox/collections.h>
#include <rox/flags.h>

typedef enum RoxFreeze {
    RoxFreezeUntilLaunch = 1,
    RoxFreezeUntilForeground = 2,
    RoxFreezeNone = 3
} RoxFreeze;

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_flag_with_freeze(const char *name, bool default_value, RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string_with_freeze(
        const char *name,
        const char *default_value,
        RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value May be <code>NULL</code>. If passed, value is copied internally.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_string_with_freeze_and_options(
        const char *name,
        const char *default_value,
        RoxList *options,
        RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int_with_freeze(const char *name, int default_value, RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_int_with_freeze_and_options(
        const char *name,
        int default_value,
        RoxList *options,
        RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double_with_freeze(const char *name, double default_value, RoxFreeze freeze);

/**
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>. Value is copied internally.
 * @param default_value Default flag value.
 * @param options May be <code>NULL</code>. If passed, ownership is delegated to ROX.
 * @return Not <code>NULL</code>. Memory is managed by ROX.
 */
ROX_API RoxStringBase *rox_add_double_with_freeze_and_options(
        const char *name,
        double default_value,
        RoxList *options,
        RoxFreeze freeze);

/**
 * @param flag Not <code>NULL</code>.
 * @param freeze
 */
ROX_API void rox_freeze_flag(RoxStringBase *flag, RoxFreeze freeze);

/**
 * @param flag Not <code>NULL</code>.
 * @param freeze
 */
ROX_API void rox_unfreeze_flag(RoxStringBase *flag, RoxFreeze freeze);

/**
 * Unfreeze all flags.
 */
ROX_API void rox_unfreeze();

/**
 * Unfreeze all flags in namespace <code>ns</code>.
 */
ROX_API void rox_unfreeze_ns(const char *ns);
