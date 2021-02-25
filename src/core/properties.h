#pragma once

#include <cjson/cJSON.h>
#include "rox/server.h"
#include "context.h"

//
// CustomPropertyType
//

typedef struct CustomPropertyType {
    const char *const type;
    const char *const external_type;
} CustomPropertyType;

ROX_INTERNAL extern const CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_STRING;
ROX_INTERNAL extern const CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_BOOL;
ROX_INTERNAL extern const CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_INT;
ROX_INTERNAL extern const CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_DOUBLE;
ROX_INTERNAL extern const CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_SEMVER;

//
// CustomProperty
//

typedef struct CustomProperty CustomProperty;

/**
 * @param name Not <code>NULL</code>.
 * @param type Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 * @return
 */
ROX_INTERNAL CustomProperty *custom_property_create(
        const char *name,
        const CustomPropertyType *type,
        void *target,
        rox_custom_property_value_generator generator);

/**
 * @param name Not <code>NULL</code>.
 * @param type Not <code>NULL</code>.
 * @param value Not <code>NULL</code>. The ownership is delegated to the returned <code>CustomProperty</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL CustomProperty *custom_property_create_using_value(
        const char *name,
        const CustomPropertyType *type,
        RoxDynamicValue *value);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *custom_property_get_name(CustomProperty *property);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL const CustomPropertyType *custom_property_get_type(CustomProperty *property);

/**
 * The returned value must be freed after use by the caller
 * by invoking <code>dynamic_value_free()</code>.
 *
 * @param property Not <code>NULL</code>.
 */
ROX_INTERNAL RoxDynamicValue *custom_property_get_value(CustomProperty *property, RoxContext *context);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>. Must be freed after use.
 */
cJSON *custom_property_to_json(CustomProperty *property);

/**
 * @param property Not <code>NULL</code>.
 */
ROX_INTERNAL void custom_property_free(CustomProperty *property);

//
// DeviceProperty
//

ROX_INTERNAL CustomProperty *device_property_create(
        const char *suffix,
        const CustomPropertyType *type,
        void *target,
        rox_custom_property_value_generator generator);

ROX_INTERNAL CustomProperty *device_property_create_using_value(
        const char *suffix,
        const CustomPropertyType *type,
        RoxDynamicValue *value);

//
// DynamicProperties
//

typedef struct DynamicProperties DynamicProperties;

DynamicProperties *dynamic_properties_create();

/**
 * @param properties Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
ROX_INTERNAL void dynamic_properties_set_rule(
        DynamicProperties *properties,
        void *target,
        rox_dynamic_properties_rule rule);

/**
 * @param properties Not <code>NULL</code>.
 * @param prop_name Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL RoxDynamicValue *dynamic_properties_invoke(
        DynamicProperties *properties,
        const char *prop_name,
        RoxContext *context);

void dynamic_properties_free(DynamicProperties *properties);