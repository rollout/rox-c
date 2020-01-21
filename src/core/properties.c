#include <assert.h>
#include "core/properties.h"
#include "util.h"

//
// CustomPropertyType
//

static const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_STRING = {"string", "String"};
static const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_ROX_BOOL = {"bool", "Boolean"};
static const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_INT = {"int", "Number"};
static const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_DOUBLE = {"double", "Number"};
static const CustomPropertyType ROX_INTERNAL ROX_CUSTOM_PROPERTY_TYPE_SEMVER = {"semver", "Semver"};

//
// CustomProperty
//

struct ROX_INTERNAL CustomProperty {
    char *name;
    CustomPropertyType *type;
    void *value;
    custom_property_value_generator value_generator;
};

CustomProperty *ROX_INTERNAL custom_property_create_no_value(const char *name, CustomPropertyType *type) {
    assert(name);
    assert(type);
    CustomProperty *p = calloc(1, sizeof(CustomProperty));
    p->name = mem_copy_str(name);
    p->type = type;
    return p;
}

CustomProperty *ROX_INTERNAL custom_property_create(const char *name, CustomPropertyType *type,
                                                    custom_property_value_generator generator) {
    assert(name);
    assert(type);
    assert(generator);
    CustomProperty *p = custom_property_create_no_value(name, type);
    p->value_generator = generator;
    return p;
}

CustomProperty *
ROX_INTERNAL custom_property_create_using_value(const char *name, CustomPropertyType *type, void *value) {
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

void *ROX_INTERNAL custom_property_get_value(CustomProperty *property, Context *context) {
    assert(property);
    if (property->value_generator) {
        return property->value_generator(context);
    }
    return property->value;
}

void ROX_INTERNAL custom_property_serialize_to_json(CustomProperty *property, char *buffer, size_t buffer_size) {
    // TODO: implement
}

void ROX_INTERNAL custom_property_free(CustomProperty *property) {
    assert(property);
    free(property->name);
    free(property);
}

// TODO: implement

