#pragma once

#include "roxapi.h"
#include "core/client.h"

typedef struct ROX_INTERNAL RoxCore RoxCore;

RoxCore *ROX_INTERNAL rox_core_create();

/**
 * @param core Not <code>NULL</code>.
 * @param sdk_settings Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param rox_options May be <code>NULL</code>.
 * @return May be <code>NULL</code> in case of an invalid input. In this case, see error logs for details.
 */
bool ROX_INTERNAL rox_core_setup(
        RoxCore *core,
        SdkSettings *sdk_settings,
        DeviceProperties *device_properties,
        RoxOptions *rox_options);

/**
 * @param core Not <code>NULL</code>.
 * @param context Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_core_set_context(RoxCore *core, Context *context);

/**
 * @param core Not <code>NULL</code>.
 * @param flag Not <code>NULL</code>.
 * @param name Not <code>NULL</code>. Flag name <em>including namespace prefix</em>.
 */
void ROX_INTERNAL rox_core_add_flag(RoxCore *core, Variant *flag, const char *name);

/**
 * @param core Not <code>NULL</code>.
 * @param property Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_core_add_custom_property(RoxCore *core, CustomProperty *property);

/**
 * @param core Not <code>NULL</code>.
 * @param property Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_core_add_custom_property_if_not_exists(RoxCore *core, CustomProperty *property);

/**
 * Note the returned pointer must be freed by the caller via invoking <code>dynamic_api_free</code>.
 *
 * @param core Not <code>NULL</code>.
 * @param entities_provider Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
DynamicApi *ROX_INTERNAL rox_core_create_dynamic_api(RoxCore *core, EntitiesProvider *entities_provider);

/**
 * @param core Not <code>NULL</code>.
 */
void ROX_INTERNAL rox_core_free(RoxCore *core);
