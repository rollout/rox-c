#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "rox/server.h"
#include "util.h"
#include "collections.h"

//
// Handle
//

struct RoxDynamicValue {
    int *int_value;
    double *double_value;
    char *str_value;
    struct tm *datetime_value;
    RoxList *list_value;
    RoxMap *map_value;
    bool is_true;
    bool is_false;
    bool is_null;
    bool is_undefined;
};

//
// Constructors
//

static RoxDynamicValue *_create_value() {
    return (RoxDynamicValue *) calloc(1, sizeof(RoxDynamicValue));
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_int(int value) {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->int_value = mem_copy_int(value);
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_int_ptr(int *value) {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->int_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_double(double value) {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->double_value = mem_copy_double(value);
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_double_ptr(double *value) {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->double_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_boolean(bool value) {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->is_true = value;
    dynamic_value->is_false = !value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_string_copy(const char *value) {
    assert(value);
    return rox_dynamic_value_create_string_ptr(mem_copy_str(value));
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_string_ptr(char *value) {
    assert(value);
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->str_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_datetime_copy(const struct tm *value) {
    assert(value);
    return rox_dynamic_value_create_datetime_ptr(mem_copy_str(value));
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_datetime_ptr(struct tm *value) {
    assert(value);
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->datetime_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_list(RoxList *value) {
    assert(value);
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->list_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_map(RoxMap *value) {
    assert(value);
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->map_value = value;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_null() {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->is_null = true;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_undefined() {
    RoxDynamicValue *dynamic_value = _create_value();
    dynamic_value->is_undefined = true;
    return dynamic_value;
}

ROX_API RoxDynamicValue *rox_dynamic_value_create_copy(RoxDynamicValue *value) {
    assert(value);
    RoxDynamicValue *copy = _create_value();
    if (value->int_value) {
        copy->int_value = mem_copy_int(*value->int_value);
    }
    if (value->double_value) {
        copy->double_value = mem_copy_double(*value->double_value);
    }
    if (value->str_value) {
        copy->str_value = mem_copy_str(value->str_value);
    }
    if (value->list_value) {
        copy->list_value = rox_list_create();
        ROX_LIST_FOREACH(item, value->list_value, {
            RoxDynamicValue *dv = (RoxDynamicValue *) item;
            rox_list_add(copy->list_value, rox_dynamic_value_create_copy(dv));
        })
    }
    if (value->map_value) {
        copy->map_value = rox_map_create();
        ROX_MAP_FOREACH(key, val, value->map_value, {
            rox_map_add(copy->map_value,
                        mem_copy_str(key),
                        rox_dynamic_value_create_copy(val));
        })
    }
    copy->is_undefined = value->is_undefined;
    copy->is_null = value->is_null;
    copy->is_true = value->is_true;
    copy->is_false = value->is_false;
    return copy;
}

//
// Check methods
//

ROX_API bool rox_dynamic_value_is_int(RoxDynamicValue *value) {
    assert(value);
    return value->int_value != NULL;
}

ROX_API bool rox_dynamic_value_is_double(RoxDynamicValue *value) {
    assert(value);
    return value->double_value != NULL;
}

ROX_API bool rox_dynamic_value_is_boolean(RoxDynamicValue *value) {
    assert(value);
    return value->is_true || value->is_false;
}

ROX_API bool rox_dynamic_value_is_string(RoxDynamicValue *value) {
    assert(value);
    return value->str_value != NULL;
}

ROX_API bool rox_dynamic_value_is_datetime(RoxDynamicValue *value) {
    assert(value);
    return value->datetime_value != NULL;
}

ROX_API bool rox_dynamic_value_is_list(RoxDynamicValue *value) {
    assert(value);
    return value->list_value != NULL;
}

ROX_API bool rox_dynamic_value_is_map(RoxDynamicValue *value) {
    assert(value);
    return value->map_value != NULL;
}

ROX_API bool rox_dynamic_value_is_undefined(RoxDynamicValue *value) {
    assert(value);
    return value->is_undefined;
}

ROX_API bool rox_dynamic_value_is_null(RoxDynamicValue *value) {
    assert(value);
    return value->is_null;
}

//
// Getters
//

ROX_API int rox_dynamic_value_get_int(RoxDynamicValue *value) {
    assert(value);
    assert(value->int_value);
    return *value->int_value;
}

ROX_API double rox_dynamic_value_get_double(RoxDynamicValue *value) {
    assert(value);
    assert(value->double_value);
    return *value->double_value;
}

ROX_API bool rox_dynamic_value_get_boolean(RoxDynamicValue *value) {
    assert(value);
    assert(value->is_true || value->is_false);
    return value->is_true;
}

ROX_API char *rox_dynamic_value_get_string(RoxDynamicValue *value) {
    assert(value);
    assert(value->str_value);
    return value->str_value;
}

ROX_API struct tm *rox_dynamic_value_get_datetime(RoxDynamicValue *value) {
    assert(value);
    assert(value->datetime_value);
    return value->datetime_value;
}

ROX_API RoxList *rox_dynamic_value_get_list(RoxDynamicValue *value) {
    assert(value);
    assert(value->list_value);
    return value->list_value;
}

ROX_API RoxMap *rox_dynamic_value_get_map(RoxDynamicValue *value) {
    assert(value);
    assert(value->map_value);
    return value->map_value;
}

//
// Other
//

ROX_API bool rox_dynamic_value_equals(RoxDynamicValue *v1, RoxDynamicValue *v2) {
    assert(v1);
    assert(v2);
    if (v1->is_null) {
        return v2->is_null;
    } else if (v1->is_undefined) {
        return v2->is_undefined;
    } else if (v1->int_value || v1->double_value) {
        if (!v2->int_value && !v2->double_value) {
            return false;
        }
        double d1 = v1->int_value ? *v1->int_value : *v1->double_value;
        double d2 = v2->int_value ? *v2->int_value : *v2->double_value;
        return fabs(d1 - d2) < FLT_EPSILON;
    } else if (v1->is_true) {
        return v2->is_true;
    } else if (v1->is_false) {
        return v2->is_false;
    } else if (v1->str_value) {
        if (!v2->str_value) {
            return false;
        }
        return strcmp(v1->str_value, v2->str_value) == 0;
    }
    return false;
}

//
// Destructor
//

ROX_API void rox_dynamic_value_free(RoxDynamicValue *value) {
    assert(value);
    if (value->int_value) {
        free(value->int_value);
    }
    if (value->double_value) {
        free(value->double_value);
    }
    if (value->str_value) {
        free(value->str_value);
    }
    if (value->list_value) {
        rox_list_free_cb(value->list_value, (void (*)(void *)) &rox_dynamic_value_free);
    }
    if (value->map_value) {
        rox_map_free_with_keys_and_values_cb(
                value->map_value, &free,
                (void (*)(void *)) &rox_dynamic_value_free);
    }
    free(value);
}
