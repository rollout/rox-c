#include <assert.h>
#include <math.h>
#include <float.h>
#include "dynamic.h"
#include "util.h"

//
// Handle
//

struct ROX_INTERNAL DynamicValue {
    int *int_value;
    double *double_value;
    char *str_value;
    List *list_value;
    HashTable *map_value;
    bool is_true;
    bool is_false;
    bool is_null;
    bool is_undefined;
};

//
// Constructors
//

static DynamicValue *_create_value() {
    return (DynamicValue *) calloc(1, sizeof(DynamicValue));
}

DynamicValue *ROX_INTERNAL dynamic_value_create_int(int value) {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->int_value = mem_copy_int(value);
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_double(double value) {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->double_value = mem_copy_double(value);
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_double_ptr(double *value) {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->double_value = value;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_boolean(bool value) {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->is_true = value;
    dynamic_value->is_false = !value;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_string_copy(const char *value) {
    assert(value);
    return dynamic_value_create_string_ptr(mem_copy_str(value));
}

DynamicValue *ROX_INTERNAL dynamic_value_create_string_ptr(char *value) {
    assert(value);
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->str_value = value;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_list(List *value) {
    assert(value);
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->list_value = value;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_map(HashTable *value) {
    assert(value);
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->map_value = value;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_null() {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->is_null = true;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_undefined() {
    DynamicValue *dynamic_value = _create_value();
    dynamic_value->is_undefined = true;
    return dynamic_value;
}

DynamicValue *ROX_INTERNAL dynamic_value_create_copy(DynamicValue *dynamic_value) {
    assert(dynamic_value);
    DynamicValue *copy = _create_value();
    if (dynamic_value->int_value) {
        copy->int_value = mem_copy_int(*dynamic_value->int_value);
    }
    if (dynamic_value->double_value) {
        copy->double_value = mem_copy_double(*dynamic_value->double_value);
    }
    if (dynamic_value->str_value) {
        copy->str_value = mem_copy_str(dynamic_value->str_value);
    }
    if (dynamic_value->list_value) {
        list_new(&copy->list_value);
        LIST_FOREACH(item, dynamic_value->list_value, {
            DynamicValue *dv = (DynamicValue *) item;
            list_add(copy->list_value, dynamic_value_create_copy(dv));
        })
    }
    if (dynamic_value->map_value) {
        hashtable_new(&copy->map_value);
        TableEntry *entry;
        HASHTABLE_FOREACH(entry, dynamic_value->map_value, {
            hashtable_add(copy->map_value,
                          mem_copy_str(entry->key),
                          dynamic_value_create_copy(entry->value));
        })
    }
    copy->is_undefined = dynamic_value->is_undefined;
    copy->is_null = dynamic_value->is_null;
    copy->is_true = dynamic_value->is_true;
    copy->is_false = dynamic_value->is_false;
    return copy;
}

//
// Check methods
//

bool ROX_INTERNAL dynamic_value_is_int(DynamicValue *value) {
    assert(value);
    return value->int_value != NULL;
}

bool ROX_INTERNAL dynamic_value_is_double(DynamicValue *value) {
    assert(value);
    return value->double_value != NULL;
}

bool ROX_INTERNAL dynamic_value_is_boolean(DynamicValue *value) {
    assert(value);
    return value->is_true || value->is_false;
}

bool ROX_INTERNAL dynamic_value_is_string(DynamicValue *value) {
    assert(value);
    return value->str_value != NULL;
}

bool ROX_INTERNAL dynamic_value_is_list(DynamicValue *value) {
    assert(value);
    return value->list_value != NULL;
}

bool ROX_INTERNAL dynamic_value_is_map(DynamicValue *value) {
    assert(value);
    return value->map_value != NULL;
}

bool ROX_INTERNAL dynamic_value_is_undefined(DynamicValue *value) {
    assert(value);
    return value->is_undefined;
}

bool ROX_INTERNAL dynamic_value_is_null(DynamicValue *value) {
    assert(value);
    return value->is_null;
}

//
// Getters
//

int ROX_INTERNAL dynamic_value_get_int(DynamicValue *value) {
    assert(value);
    assert(value->int_value);
    return *value->int_value;
}

double ROX_INTERNAL dynamic_value_get_double(DynamicValue *value) {
    assert(value);
    assert(value->double_value);
    return *value->double_value;
}

bool ROX_INTERNAL dynamic_value_get_boolean(DynamicValue *value) {
    assert(value);
    assert(value->is_true || value->is_false);
    return value->is_true;
}

char *ROX_INTERNAL dynamic_value_get_string(DynamicValue *value) {
    assert(value);
    assert(value->str_value);
    return value->str_value;
}

List *ROX_INTERNAL dynamic_value_get_list(DynamicValue *value) {
    assert(value);
    assert(value->list_value);
    return value->list_value;
}

HashTable *ROX_INTERNAL dynamic_value_get_map(DynamicValue *value) {
    assert(value);
    assert(value->map_value);
    return value->map_value;
}

//
// Other
//

bool ROX_INTERNAL dynamic_value_equals(DynamicValue *v1, DynamicValue *v2) {
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

DynamicValue *ROX_INTERNAL dynamic_value_free(DynamicValue *value) {
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
        list_destroy_cb(value->list_value, (void (*)(void *)) &dynamic_value_free);
    }
    if (value->map_value) {
        TableEntry *entry;
        HASHTABLE_FOREACH(entry, value->map_value, {
            free(entry->key);
            dynamic_value_free(entry->value);
        })
    }
    free(value);
}
