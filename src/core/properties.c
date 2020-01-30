#include <assert.h>
#include <stdio.h>
#include "properties.h"
#include "util.h"
#include "dynamic.h"

//
// CustomPropertyType
//

const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_STRING = {"string", "String"};
const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL = {"bool", "Boolean"};
const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_INT = {"int", "Number"};
const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_DOUBLE = {"double", "Number"};
const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_SEMVER = {"semver", "Semver"};

//
// CustomProperty
//

struct ROX_INTERNAL CustomProperty {
    char *name;
    const CustomPropertyType *type;
    DynamicValue *value;
    custom_property_value_generator value_generator;
};

CustomProperty *ROX_INTERNAL custom_property_create_no_value(
        const char *name,
        const CustomPropertyType *type) {
    assert(name);
    assert(type);
    CustomProperty *p = calloc(1, sizeof(CustomProperty));
    p->name = mem_copy_str(name);
    p->type = type;
    return p;
}

CustomProperty *ROX_INTERNAL custom_property_create(
        const char *name,
        const CustomPropertyType *type,
        custom_property_value_generator generator) {
    assert(name);
    assert(type);
    assert(generator);
    CustomProperty *p = custom_property_create_no_value(name, type);
    p->value_generator = generator;
    return p;
}

CustomProperty *ROX_INTERNAL custom_property_create_using_value(
        const char *name,
        const CustomPropertyType *type,
        DynamicValue *value) {
    assert(name);
    assert(type);
    assert(value);
    CustomProperty *p = custom_property_create_no_value(name, type);
    p->value = value;
    return p;
}

char *ROX_INTERNAL custom_property_get_name(CustomProperty *property) {
    assert(property);
    return property->name;
}

const CustomPropertyType *ROX_INTERNAL custom_property_get_type(CustomProperty *property) {
    assert(property);
    return property->type;
}

DynamicValue *ROX_INTERNAL custom_property_get_value(CustomProperty *property, Context *context) {
    assert(property);
    if (property->value_generator) {
        return property->value_generator(context);
    }
    if (property->value) {
        return dynamic_value_create_copy(property->value);
    }
    return NULL;
}

void ROX_INTERNAL custom_property_serialize_to_json(CustomProperty *property, const char *buffer, size_t buffer_size) {
    assert(property);
    assert(buffer);
    assert(buffer_size > 0);
    ROX_JSON_SERIALIZE(
            buffer, buffer_size,
            "name", ROX_JSON_STRING(property->name),
            "type", ROX_JSON_STRING(property->type->type),
            "externalType", ROX_JSON_STRING(property->type->external_type));
}

void ROX_INTERNAL custom_property_free(CustomProperty *property) {
    assert(property);
    free(property->name);
    free(property);
}

//
// DeviceProperty
//

#define ROX_DEVICE_PROPERTY_NAME_BUFFER_SIZE 256

CustomProperty *ROX_INTERNAL device_property_create(
        const char *suffix,
        const CustomPropertyType *type,
        custom_property_value_generator generator) {

    assert(suffix);
    assert(type);
    assert(generator);
    char buffer[ROX_DEVICE_PROPERTY_NAME_BUFFER_SIZE];
    sprintf_s(buffer, ROX_DEVICE_PROPERTY_NAME_BUFFER_SIZE, "rox.%s", suffix);
    return custom_property_create(buffer, type, generator);
}

CustomProperty *ROX_INTERNAL device_property_create_using_value(
        const char *suffix,
        const CustomPropertyType *type,
        DynamicValue *value) {
    char buffer[ROX_DEVICE_PROPERTY_NAME_BUFFER_SIZE];
    sprintf_s(buffer, ROX_DEVICE_PROPERTY_NAME_BUFFER_SIZE, "rox.%s", suffix);
    return custom_property_create_using_value(buffer, type, value);
}

//
// DynamicProperties
//

struct DynamicValue *ROX_INTERNAL default_dynamic_properties_rule(const char *prop_name, Context *context) {
    assert(prop_name);
    return context != NULL ? context_get(context, prop_name) : NULL;
}

struct ROX_INTERNAL DynamicProperties {
    dynamic_properties_rule rule;
};

DynamicProperties *dynamic_properties_create() {
    DynamicProperties *properties = calloc(1, sizeof(DynamicProperties));
    properties->rule = &default_dynamic_properties_rule;
    return properties;
}

void ROX_INTERNAL dynamic_properties_set_rule(
        DynamicProperties *properties,
        dynamic_properties_rule rule) {
    assert(properties);
    assert(rule);
    properties->rule = rule;
}

dynamic_properties_rule ROX_INTERNAL dynamic_properties_get_rule(DynamicProperties *properties) {
    assert(properties);
    return properties->rule;
}

void dynamic_properties_free(DynamicProperties *properties) {
    assert(properties);
    free(properties);
}