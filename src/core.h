#pragma once

#include "rox/errors.h"
#include "core/client.h"
#include "core/network.h"
#include "core/configuration.h"

typedef struct RoxCore RoxCore;

/**
 * @param request_config May be <code>NULL</code>. Used for testing.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxCore *rox_core_create(RequestConfig *request_config);

/**
 * @param core Not <code>NULL</code>.
 * @param sdk_settings Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param rox_options May be <code>NULL</code>.
 */
ROX_INTERNAL RoxStateCode rox_core_setup(
        RoxCore *core,
        SdkSettings *sdk_settings,
        DeviceProperties *device_properties,
        RoxOptions *rox_options);

/**
 * @param core Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_core_fetch(RoxCore *core, bool is_source_pushing);

/**
 * @param core Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_INTERNAL void rox_core_set_context(RoxCore *core, RoxContext *context);

/**
 * @param core Not <code>NULL</code>.
 * @param flag Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>.
 */
ROX_INTERNAL void rox_core_add_flag(RoxCore *core, RoxStringBase *flag, const char *name);

/**
 * @param core Not <code>NULL</code>.
 * @param property Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_core_add_custom_property(RoxCore *core, CustomProperty *property);

/**
 * @param core Not <code>NULL</code>.
 * @param property Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_core_add_custom_property_if_not_exists(RoxCore *core, CustomProperty *property);

/**
 * Note the returned pointer must be freed by the caller via invoking <code>rox_dynamic_api_free</code>.
 *
 * @param core Not <code>NULL</code>.
 * @param entities_provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxDynamicApi *rox_core_create_dynamic_api(RoxCore *core, EntitiesProvider *entities_provider);

/**
 * @param core Not <code>NULL</code>.
 */
ROX_INTERNAL void rox_core_free(RoxCore *core);

/**
 * @param core Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL FlagRepository *rox_core_get_flag_repository(RoxCore *core);

/**
 * @param core Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL ConfigurationFetchedInvoker *rox_core_get_configuration_fetched_invoker(RoxCore *core);

//
// PeriodicTask - Timer for periodic operations
//

typedef struct PeriodicTask PeriodicTask;

/**
 * Callback function type for periodic tasks.
 * @param target User-provided context pointer.
 */
typedef void (*periodic_task_func)(void *target);

/**
 * Create and start a periodic task that calls the given function at regular intervals.
 * @param seconds Interval in seconds (must be > 0).
 * @param target User-provided context pointer (passed to callback).
 * @param func Callback function to call periodically. Not <code>NULL</code>.
 * @return Not <code>NULL</code>. Periodic task handle.
 */
ROX_INTERNAL PeriodicTask *periodic_task_create(int seconds, void *target, periodic_task_func func);

/**
 * Stop and free a periodic task.
 * @param task Not <code>NULL</code>.
 */
ROX_INTERNAL void periodic_task_free(PeriodicTask *task);

