#pragma once

#include <cjson/cJSON.h>
#include "roxapi.h"
#include "context.h"
#include "dynamic.h"

//
// CustomPropertyType
//

typedef struct ROX_INTERNAL CustomPropertyType {
    const char *const type;
    const char *const external_type;
} CustomPropertyType;

extern const ROX_INTERNAL CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_STRING;
extern const ROX_INTERNAL CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL;
extern const ROX_INTERNAL CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_INT;
extern const ROX_INTERNAL CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_DOUBLE;
extern const ROX_INTERNAL CustomPropertyType ROX_CUSTOM_PROPERTY_TYPE_SEMVER;

//
// CustomProperty
//

typedef ROX_INTERNAL DynamicValue *(*custom_property_value_generator)(void *target, Context *context);

typedef struct ROX_INTERNAL CustomProperty CustomProperty;

/**
 * @param name Not <code>NULL</code>.
 * @param type Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param generator Not <code>NULL</code>.
 * @return
 */
CustomProperty *ROX_INTERNAL custom_property_create(
        const char *name,
        const CustomPropertyType *type,
        void *target,
        custom_property_value_generator generator);

CustomProperty *ROX_INTERNAL custom_property_create_using_value(
        const char *name,
        const CustomPropertyType *type,
        DynamicValue *value);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
char *ROX_INTERNAL custom_property_get_name(CustomProperty *property);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
const CustomPropertyType *ROX_INTERNAL custom_property_get_type(CustomProperty *property);

/**
 * The returned value must be freed after use by the caller
 * by invoking <code>dynamic_value_free()</code>.
 *
 * @param property Not <code>NULL</code>.
 */
DynamicValue *ROX_INTERNAL custom_property_get_value(CustomProperty *property, Context *context);

/**
 * @param property Not <code>NULL</code>.
 * @return Not <code>NULL</code>. Must be freed after use.
 */
cJSON *custom_property_to_json(CustomProperty *property);

/**
 * @param property Not <code>NULL</code>.
 */
void ROX_INTERNAL custom_property_free(CustomProperty *property);

//
// DeviceProperty
//

CustomProperty *ROX_INTERNAL device_property_create(
        const char *suffix,
        const CustomPropertyType *type,
        void *target,
        custom_property_value_generator generator);

CustomProperty *ROX_INTERNAL device_property_create_using_value(
        const char *suffix,
        const CustomPropertyType *type,
        DynamicValue *value);

//
// DynamicProperties
//

typedef struct ROX_INTERNAL DynamicProperties DynamicProperties;

/**
 * Note the returned value, if not <code>NULL</code>, must be freed by the caller
 * by invoking <code>dynamic_value_free()</code>
 */
typedef ROX_INTERNAL DynamicValue *(*dynamic_properties_rule)(
        const char *prop_name,
        void *target,
        Context *context);

DynamicProperties *dynamic_properties_create();

/**
 * @param properties Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
void ROX_INTERNAL dynamic_properties_set_rule(
        DynamicProperties *properties,
        void *target,
        dynamic_properties_rule rule);

/**
 * @param properties Not <code>NULL</code>.
 * @param prop_name Not <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
DynamicValue *ROX_INTERNAL dynamic_properties_invoke(
        DynamicProperties *properties,
        const char *prop_name,
        Context *context);

void dynamic_properties_free(DynamicProperties *properties);