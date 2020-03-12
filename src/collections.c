#include "collections.h"
#include <stdarg.h>
#include <assert.h>
#include <collectc/list.h>
#include <collectc/hashset.h>
#include <collectc/hashtable.h>
#include "util.h"

struct RoxMap {
    HashTable *map;
};

struct RoxList {
    List *list;
};

struct RoxSet {
    HashSet *set;
};

struct RoxListIter {
    ListIter iter;
};

struct RoxSetIter {
    HashSetIter iter;
};

struct RoxMapIter {
    HashTableIter iter;
};

ROX_INTERNAL RoxMap *rox_map_create() {
    RoxMap *map = calloc(1, sizeof(RoxMap));
    hashtable_new(&map->map);
    return map;
}

ROX_INTERNAL void rox_map_free(RoxMap *map) {
    assert(map);
    hashtable_destroy(map->map);
    free(map);
}

ROX_INTERNAL RoxSet *rox_set_create() {
    RoxSet *set = calloc(1, sizeof(RoxSet));
    hashset_new(&set->set);
    return set;
}

ROX_INTERNAL void rox_set_free(RoxSet *set) {
    assert(set);
    hashset_destroy(set->set);
    free(set);
}

ROX_INTERNAL RoxList *rox_list_create() {
    RoxList *list = calloc(1, sizeof(RoxList));
    list_new(&list->list);
    return list;
}

ROX_INTERNAL void rox_list_free(RoxList *list) {
    assert(list);
    list_destroy(list->list);
    free(list);
}

ROX_INTERNAL RoxMapIter *rox_map_iter_create() {
    return calloc(1, sizeof(RoxMapIter));
}

ROX_INTERNAL void rox_map_iter_free(RoxMapIter *iter) {
    assert(iter);
    free(iter);
}

ROX_INTERNAL RoxSetIter *rox_set_iter_create() {
    return calloc(1, sizeof(RoxSetIter));
}

ROX_INTERNAL void rox_set_iter_free(RoxSetIter *iter) {
    assert(iter);
    free(iter);
}

ROX_INTERNAL RoxListIter *rox_list_iter_create() {
    return calloc(1, sizeof(RoxListIter));
}

ROX_INTERNAL void rox_list_iter_free(RoxListIter *iter) {
    assert(iter);
    free(iter);
}

ROX_INTERNAL void rox_list_free_cb(RoxList *list, void (*cb)(void *)) {
    assert(list);
    list_destroy_cb(list->list, cb);
    free(list);
}

ROX_INTERNAL bool rox_map_add(RoxMap *map, void *key, void *val) {
    assert(map);
    assert(key);
    return hashtable_add(map->map, key, val) == CC_OK;
}

ROX_INTERNAL bool rox_map_remove(RoxMap *map, void *key, void **out) {
    assert(map);
    assert(key);
    assert(out);
    return hashtable_remove(map->map, key, out) == CC_OK;
}

ROX_INTERNAL bool rox_map_contains_key(RoxMap *map, void *key) {
    assert(map);
    assert(key);
    return hashtable_contains_key(map->map, key);
}

ROX_INTERNAL bool rox_map_get(RoxMap *map, void *key, void **out) {
    assert(map);
    assert(key);
    assert(out);
    return hashtable_get(map->map, key, out) == CC_OK;
}

ROX_INTERNAL bool rox_list_add(RoxList *list, void *element) {
    assert(list);
    return list_add(list->list, element) == CC_OK;
}

ROX_INTERNAL bool rox_set_add(RoxSet *set, void *element) {
    assert(set);
    return hashset_add(set->set, element) == CC_OK;
}

ROX_INTERNAL size_t rox_list_size(RoxList *list) {
    assert(list);
    return list_size(list->list);
}

ROX_INTERNAL size_t rox_set_size(RoxSet *set) {
    assert(set);
    return hashset_size(set->set);
}

ROX_INTERNAL size_t rox_map_size(RoxMap *map) {
    assert(map);
    return hashtable_size(map->map);
}

ROX_INTERNAL bool rox_list_sort(RoxList *list, int (*cmp)(void const *, void const *)) {
    assert(list);
    return list_sort(list->list, cmp) == CC_OK;
}

ROX_INTERNAL void rox_list_reverse(RoxList *list) {
    assert(list);
    list_reverse(list->list);
}

ROX_INTERNAL bool rox_list_get_at(RoxList *list, size_t index, void **out) {
    assert(list);
    assert(index >= 0);
    assert(out);
    return list_get_at(list->list, index, out) == CC_OK;
}

ROX_INTERNAL bool rox_list_get_first(RoxList *list, void **out) {
    assert(list);
    assert(out);
    return list_get_first(list->list, out) == CC_OK;
}

ROX_INTERNAL bool rox_list_remove(RoxList *list, void *element) {
    assert(list);
    assert(element);
    return list_remove(list->list, element, NULL) == CC_OK;
}

ROX_INTERNAL bool rox_list_remove_all(RoxList *list) {
    assert(list);
    return list_remove_all(list->list) == CC_OK;
}

ROX_INTERNAL bool rox_set_contains(RoxSet *set, void *element) {
    assert(set);
    assert(element);
    return hashset_contains(set->set, element);
}

ROX_INTERNAL void rox_list_iter_init(RoxListIter *iter, RoxList *list) {
    assert(iter);
    assert(list);
    list_iter_init(&iter->iter, list->list);
}

ROX_INTERNAL bool rox_list_iter_next(RoxListIter *iter, void **out) {
    assert(iter);
    assert(out);
    return list_iter_next(&iter->iter, out) != CC_ITER_END;
}

ROX_INTERNAL void rox_map_iter_init(RoxMapIter *iter, RoxMap *map) {
    assert(iter);
    assert(map);
    hashtable_iter_init(&iter->iter, map->map);
}

ROX_INTERNAL bool rox_map_iter_next(RoxMapIter *iter, void **key, void **value) {
    assert(iter);
    assert(key);
    assert(value);
    TableEntry *entry;
    if (hashtable_iter_next(&iter->iter, &entry) != CC_ITER_END) {
        *key = entry->key;
        *value = entry->value;
        return true;
    }
    return false;
}

ROX_INTERNAL void rox_set_iter_init(RoxSetIter *iter, RoxSet *set) {
    assert(iter);
    assert(set);
    hashset_iter_init(&iter->iter, set->set);
}

ROX_INTERNAL bool rox_set_iter_next(RoxSetIter *iter, void **out) {
    assert(iter);
    assert(out);
    return hashset_iter_next(&iter->iter, out) != CC_ITER_END;
}

ROX_INTERNAL RoxMap *mem_copy_map(RoxMap *map) {
    assert(map);
    RoxMap *params = rox_map_create();
    ROX_MAP_FOREACH(key, value, map, {
        hashtable_add(params->map, key, value);
    })
    return params;
}

ROX_INTERNAL RoxList *mem_copy_list(RoxList *list) {
    assert(list);
    RoxList *copy = calloc(1, sizeof(RoxList));
    list_copy_shallow(list->list, &copy->list);
    return copy;
}

ROX_INTERNAL RoxList *mem_deep_copy_list(RoxList *list, void *(*copy_func)(void *)) {
    assert(list);
    assert(copy_func);
    RoxList *copy = calloc(1, sizeof(RoxList));
    list_copy_deep(list->list, copy_func, &copy->list);
    return copy;
}

ROX_INTERNAL RoxSet *mem_copy_set(RoxSet *set) {
    assert(set);
    RoxSet *copy = rox_set_create();
    ROX_SET_FOREACH(item, set, {
        hashset_add(copy->set, item);
    })
    return copy;
}

ROX_INTERNAL RoxSet *mem_deep_copy_set(RoxSet *set, void *(*copy_func)(void *)) {
    assert(set);
    RoxSet *copy = rox_set_create();
    ROX_SET_FOREACH(item, set, {
        hashset_add(copy->set, copy_func(item));
    })
    return copy;
}

ROX_INTERNAL RoxMap *mem_deep_copy_str_value_map(RoxMap *map) {
    assert(map);
    RoxMap *copy = rox_map_create();
    ROX_MAP_FOREACH(key, value, map, {
        hashtable_add(copy->map, key, mem_copy_str(value));
    })
    return copy;
}

ROX_API RoxList *rox_list_create_va(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    RoxList *list = rox_list_create();
    void *item = va_arg(args, void*);
    while (item != NULL) {
        rox_list_add(list, item);
        item = va_arg(args, void*);
    };
            va_end(args);
    return list;
}

ROX_API RoxList *rox_list_create_str_va(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    RoxList *list = rox_list_create();
    char *item = va_arg(args, char*);
    while (item != NULL) {
        rox_list_add(list, mem_copy_str(item));
        item = va_arg(args, char*);
    };
            va_end(args);
    return list;
}

ROX_INTERNAL bool list_equals(RoxList *one, RoxList *another, bool (*cmp)(void *v1, void *v2)) {
    assert(one);
    assert(another);
    if (rox_list_size(one) != rox_list_size(another)) {
        return false;
    }
    RoxListIter i1, i2;
    rox_list_iter_init(&i1, one);
    rox_list_iter_init(&i2, another);
    void *v1, *v2;
    while (rox_list_iter_next(&i1, &v1) && rox_list_iter_next(&i2, &v2)) {
        if (!cmp(v1, v2)) {
            return false;
        }
    }
    return true;
}

ROX_INTERNAL bool str_list_equals(RoxList *one, RoxList *another) {
    assert(one);
    assert(another);
    return list_equals(one, another, (bool (*)(void *, void *)) &str_equals);
}

ROX_API RoxSet *rox_set_create_va(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    RoxSet *set = rox_set_create();
    void *item = va_arg(args, void*);
    while (item != NULL) {
        rox_set_add(set, item);
        item = va_arg(args, void*);
    };
            va_end(args);
    return set;
}

ROX_API RoxMap *rox_map_create_va(void *skip, ...) {
    va_list args;
            va_start(args, skip);

    RoxMap *map = rox_map_create();
    char *property_name = va_arg(args, char*);
    while (property_name != NULL) {
        void *property_value = va_arg(args, void *);
        rox_map_add(map, property_name, property_value);
        property_name = va_arg(args, char*);
    };
            va_end(args);
    return map;
}

ROX_INTERNAL void rox_map_free_with_values(RoxMap *map) {
    assert(map);
    rox_map_free_with_values_cb(map, &free);
}

ROX_INTERNAL void rox_map_free_with_keys_and_values(RoxMap *map) {
    assert(map);
    rox_map_free_with_keys_and_values_cb(map, &free, &free);
}

ROX_INTERNAL void rox_map_free_with_values_cb(RoxMap *map, void (*f)(void *)) {
    assert(map);
    TableEntry *entry;
    ROX_MAP_FOREACH(key, value, map, {
        f(value);
    })
    hashtable_destroy(map->map);
    free(map);
}

ROX_INTERNAL void rox_map_free_with_keys_and_values_cb(
        RoxMap *map, void (*f_key)(void *), void (*f_value)(void *)) {
    assert(map);
    ROX_MAP_FOREACH(key, value, map, {
        f_key(key);
        if (value) {
            f_value(value);
        }
    })
    hashtable_destroy(map->map);
    free(map);
}

ROX_INTERNAL bool str_in_list(const char *str, RoxList *list_of_strings) {
    assert(str);
    assert(list_of_strings);
    return list_contains_value(list_of_strings->list,
                               (void *) str,
                               (int (*)(const void *, const void *)) &strcmp);
}

ROX_INTERNAL char *mem_str_join(const char *separator, RoxList *strings) {
    assert(separator);
    assert(strings);
    size_t result_len = 0;
    int count = 0;
    ROX_LIST_FOREACH(item, strings, {
        char *str = (char *) item;
        result_len += strlen(str);
        ++count;
    })
    if (count == 0) {
        return mem_copy_str("");
    }
    size_t separator_len = strlen(separator);
    result_len += separator_len * (count - 1);
    char *result = malloc((result_len + 1) * sizeof(char));
    result[result_len] = '\0';
    char *dest = result;
    ROX_LIST_FOREACH(item, strings, {
        char *str = (char *) item;
        int len = strlen(str);
        if (len > 0) {
            strncpy(dest, str, len);
            dest += len;
        }
        if (--count > 0) {
            if (separator_len > 0) {
                strncpy(dest, separator, separator_len);
                dest += separator_len;
            }
        }
    })
    return result;
}
