#pragma once

#include <stdbool.h>
#include <collectc/list.h>
#include <collectc/hashtable.h>
#include "roxapi.h"

//
// Handle
//

typedef struct ROX_INTERNAL DynamicValue DynamicValue;

//
// Constructors
//

DynamicValue *ROX_INTERNAL dynamic_value_create_int(int value);

DynamicValue *ROX_INTERNAL dynamic_value_create_double(double value);

DynamicValue *ROX_INTERNAL dynamic_value_create_boolean(bool value);

/**
 * Note: the given string will be copied internally.
 * The caller is responsible for freeing it after use.
 */
DynamicValue *ROX_INTERNAL dynamic_value_create_string_copy(const char *value);

/**
 * Note: the given string will be destroyed in <code>dynamic_value_free()</code>.
 */
DynamicValue *ROX_INTERNAL dynamic_value_create_string_ptr(char *value);

/**
 * Note: the list is passed by reference.
 * The caller is responsible for destroying it after use.
 */
DynamicValue *ROX_INTERNAL dynamic_value_create_list(List *value);

/**
 * Note: the map is passed by reference.
 * The caller is responsible for destroying it after use.
 */
DynamicValue *ROX_INTERNAL dynamic_value_create_map(HashTable *value);

DynamicValue *ROX_INTERNAL dynamic_value_create_null();

DynamicValue *ROX_INTERNAL dynamic_value_create_undefined();

DynamicValue *ROX_INTERNAL dynamic_value_create_copy(DynamicValue *value);

//
// Check methods
//

bool ROX_INTERNAL dynamic_value_is_int(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_double(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_boolean(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_string(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_list(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_map(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_undefined(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_is_null(DynamicValue *value);

//
// Getters
//

int ROX_INTERNAL dynamic_value_get_int(DynamicValue *value);

double ROX_INTERNAL dynamic_value_get_double(DynamicValue *value);

bool ROX_INTERNAL dynamic_value_get_boolean(DynamicValue *value);

char *ROX_INTERNAL dynamic_value_get_string(DynamicValue *value);

List *ROX_INTERNAL dynamic_value_get_list(DynamicValue *value);

HashTable *ROX_INTERNAL dynamic_value_get_map(DynamicValue *value);

//
// Destructor
//

DynamicValue *ROX_INTERNAL dynamic_value_free(DynamicValue *value);
