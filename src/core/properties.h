#pragma once

#include "roxapi.h"
#include "context.h"

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

typedef void *ROX_INTERNAL (*custom_property_value_generator)(Context */*NULLABLE*/context);

typedef struct ROX_INTERNAL CustomProperty CustomProperty;

CustomProperty *ROX_INTERNAL custom_property_create(
        const char *name,
        const CustomPropertyType *type,
        custom_property_value_generator generator);

CustomProperty *ROX_INTERNAL custom_property_create_using_value(
        const char *name,
        const CustomPropertyType *type,
        void *value);

/**
 * @param property NOT NULL.
 * @return NOT NULL.
 */
char *ROX_INTERNAL custom_property_get_name(CustomProperty *property);

/**
 * @param property NOT NULL.
 * @return NOT NULL.
 */
const CustomPropertyType *ROX_INTERNAL custom_property_get_type(CustomProperty *property);

/**
 * @param property NOT NULL.
 */
void *ROX_INTERNAL custom_property_get_value(CustomProperty *property, Context *context);

/**
 * @param property NOT NULL.
 * @param buffer Output buffer. NOT NULL.
 * @param buffer_size MAX output size.
 * @return Newly created JSON string. NOT NULL.
 */
void custom_property_serialize_to_json(CustomProperty *property, const char *buffer, size_t buffer_size);

/**
 * @param property NOT NULL.
 */
void ROX_INTERNAL custom_property_free(CustomProperty *property);

//
// DeviceProperty
//

CustomProperty *ROX_INTERNAL device_property_create(
        const char *suffix,
        const CustomPropertyType *type,
        custom_property_value_generator generator);

CustomProperty *ROX_INTERNAL device_property_create_using_value(
        const char *suffix,
        const CustomPropertyType *type,
        void *value);

//
// DynamicProperties
//

typedef struct ROX_INTERNAL DynamicProperties DynamicProperties;

typedef void *ROX_INTERNAL (*dynamic_properties_rule)(const char *prop_name, Context */*NULLABLE*/context);

DynamicProperties *dynamic_properties_create();

/**
 * @param properties NOT NULL.
 * @param rule NOT NULL.
 */
void ROX_INTERNAL dynamic_properties_set_rule(
        DynamicProperties *properties,
        dynamic_properties_rule rule);

/**
 * @param properties NOT NULL.
 * @return NOT NULL.
 */
dynamic_properties_rule ROX_INTERNAL dynamic_properties_get_rule(DynamicProperties *properties);

void dynamic_properties_free(DynamicProperties *properties);