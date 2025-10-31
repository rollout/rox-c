#pragma once

#include "rox/defs.h"
#include "rox/collections.h"
#include <stdbool.h>
#include <time.h>

//
// DynamicValue
//

typedef struct RoxDynamicValue RoxDynamicValue;

ROX_API RoxDynamicValue *rox_dynamic_value_create_int(int value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_int_ptr(int *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_double(double value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_double_ptr(double *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_boolean(bool value);

/**
 * Note: the given string will be copied internally.
 * The caller is responsible for freeing it after use.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_string_copy(const char *value);

/**
 * Note: the given string will be destroyed in <code>dynamic_value_free()</code>.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_string_ptr(char *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_datetime_copy(const struct tm *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_datetime_ptr(struct tm *value);

/**
 * Note: the ownership of the list is delegated to the dynamic value
 * so all the memory will be freed by <code>dynamic_value_free</code>.
 *
 * @param value List of <code>RoxDynamicValue*</code>
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_list(RoxList *value);

/**
 * Note: the ownership of the map is delegated to the dynamic value
 * so all the memory including both keys and values
 * will be freed by <code>dynamic_value_free</code>.
 *
 * @param value Keys are <code>char *</code>s and values are <code>RoxDynamicValue*</code>s.
 */
ROX_API RoxDynamicValue *rox_dynamic_value_create_map(RoxMap *value);

ROX_API RoxDynamicValue *rox_dynamic_value_create_null();

ROX_API RoxDynamicValue *rox_dynamic_value_create_undefined();

ROX_API RoxDynamicValue *rox_dynamic_value_create_copy(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_int(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_double(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_boolean(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_string(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_datetime(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_list(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_map(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_undefined(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_is_null(RoxDynamicValue *value);

ROX_API int rox_dynamic_value_get_int(RoxDynamicValue *value);

ROX_API double rox_dynamic_value_get_double(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_get_boolean(RoxDynamicValue *value);

ROX_API char *rox_dynamic_value_get_string(RoxDynamicValue *value);

ROX_API struct tm *rox_dynamic_value_get_datetime(RoxDynamicValue *value);

ROX_API RoxList *rox_dynamic_value_get_list(RoxDynamicValue *value);

ROX_API RoxMap *rox_dynamic_value_get_map(RoxDynamicValue *value);

ROX_API bool rox_dynamic_value_equals(RoxDynamicValue *v1, RoxDynamicValue *v2);

ROX_API void rox_dynamic_value_free(RoxDynamicValue *value);