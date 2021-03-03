#pragma once

#include "rox/collections.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct RoxListIter RoxListIter;

typedef struct RoxSetIter RoxSetIter;

typedef struct RoxMapIter RoxMapIter;

ROX_INTERNAL RoxMap *rox_map_create();

ROX_INTERNAL void rox_map_free(RoxMap *map);

ROX_INTERNAL void rox_map_free_with_values(RoxMap *map);

ROX_INTERNAL void rox_map_free_with_values_cb(RoxMap *map, void (*f)(void *));

ROX_INTERNAL void rox_map_free_with_keys_and_values(RoxMap *map);

ROX_INTERNAL void rox_map_free_with_keys_and_values_cb(
        RoxMap *map, void (*f_key)(void *), void (*f_value)(void *));

ROX_INTERNAL RoxSet *rox_set_create();

ROX_INTERNAL void rox_set_free(RoxSet *set);

ROX_INTERNAL RoxList *rox_list_create();

ROX_INTERNAL void rox_list_free(RoxList *list);

ROX_INTERNAL RoxMapIter *rox_map_iter_create();

ROX_INTERNAL void rox_map_iter_free(RoxMapIter *iter);

ROX_INTERNAL RoxSetIter *rox_set_iter_create();

ROX_INTERNAL void rox_set_iter_free(RoxSetIter *iter);

ROX_INTERNAL RoxListIter *rox_list_iter_create();

ROX_INTERNAL void rox_list_iter_free(RoxListIter *iter);

ROX_INTERNAL void rox_list_free_cb(RoxList *list, void (*cb)(void *));

ROX_INTERNAL bool rox_map_add(RoxMap *map, void *key, void *val);

ROX_INTERNAL bool rox_map_remove(RoxMap *map, void *key, void **out);

ROX_INTERNAL bool rox_map_contains_key(RoxMap *map, void *key);

ROX_INTERNAL bool rox_map_get(RoxMap *map, void *key, void **out);

ROX_INTERNAL bool rox_list_add(RoxList *list, void *element);

ROX_INTERNAL bool rox_set_add(RoxSet *set, void *element);

ROX_INTERNAL size_t rox_list_size(RoxList *list);

ROX_INTERNAL size_t rox_set_size(RoxSet *set);

ROX_INTERNAL size_t rox_map_size(RoxMap *map);

ROX_INTERNAL bool rox_list_sort(RoxList *list, int (*cmp)(void const *, void const *));

ROX_INTERNAL void rox_list_reverse(RoxList *list);

ROX_INTERNAL bool rox_list_get_at(RoxList *list, size_t index, void **out);

ROX_INTERNAL bool rox_list_get_first(RoxList *list, void **out);

ROX_INTERNAL bool rox_list_remove(RoxList *list, void *element);

ROX_INTERNAL bool rox_list_remove_all(RoxList *list);

ROX_INTERNAL bool rox_set_contains(RoxSet *set, void *element);

ROX_INTERNAL void rox_list_iter_init(RoxListIter *iter, RoxList *list);

ROX_INTERNAL bool rox_list_iter_next(RoxListIter *iter, void **out);

ROX_INTERNAL void rox_map_iter_init(RoxMapIter *iter, RoxMap *map);

ROX_INTERNAL bool rox_map_iter_next(RoxMapIter *iter, void **key, void **value);

ROX_INTERNAL void rox_set_iter_init(RoxSetIter *iter, RoxSet *set);

ROX_INTERNAL bool rox_set_iter_next(RoxSetIter *iter, void **out);

ROX_INTERNAL RoxMap *mem_copy_map(RoxMap *map);

ROX_INTERNAL RoxSet *mem_copy_set(RoxSet *set);

ROX_INTERNAL RoxSet *mem_deep_copy_set(RoxSet *set, void *(*copy_func)(void *));

ROX_INTERNAL RoxMap *mem_deep_copy_str_value_map(RoxMap *map);

ROX_INTERNAL RoxList *mem_copy_list(RoxList *list);

ROX_INTERNAL RoxList *mem_deep_copy_list(RoxList *list, void *(*copy_func)(void *));

ROX_INTERNAL bool list_equals(RoxList *one, RoxList *another, bool (*cmp)(void *v1, void *v2));

ROX_INTERNAL bool str_list_equals(RoxList *one, RoxList *another);

ROX_INTERNAL bool str_in_list(const char *str, RoxList *list_of_strings);

/**
 * NOTE: THE RETURNED STR MUST BE FREED AFTER USE
 * @param separator Separator string. Not <code>NULL</code>.
 * @param strings List of <code>char *</code>. Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL char *mem_str_join(const char *separator, RoxList *strings);

#define ROX_SET_FOREACH(val, set, body)                                \
    {                                                                  \
        RoxSetIter* set_iter_53d46d2a04458e7b = rox_set_iter_create(); \
        rox_set_iter_init(set_iter_53d46d2a04458e7b, set);             \
        void *val;                                                     \
        while (rox_set_iter_next(set_iter_53d46d2a04458e7b, &val))     \
            body                                                       \
        rox_set_iter_free(set_iter_53d46d2a04458e7b);                  \
    }

#define ROX_MAP_FOREACH(key, value, map, body)                         \
    {                                                                  \
        RoxMapIter* map_iter_53d46d2a04458e7b = rox_map_iter_create(); \
        rox_map_iter_init(map_iter_53d46d2a04458e7b, map);             \
        void *key;                                                     \
        void *value;                                                   \
        while (rox_map_iter_next(map_iter_53d46d2a04458e7b, &key, &value))  \
            body                                                       \
        rox_map_iter_free(map_iter_53d46d2a04458e7b);                  \
    }

#define ROX_LIST_FOREACH(val, list, body)                               \
    {                                                                   \
        RoxListIter* list_iter_53d46d2a04458e7b = rox_list_iter_create(); \
        rox_list_iter_init(list_iter_53d46d2a04458e7b, list);           \
        void *val;                                                      \
        while (rox_list_iter_next(list_iter_53d46d2a04458e7b, &val))    \
            body                                                        \
        rox_list_iter_free(list_iter_53d46d2a04458e7b);                 \
    }

#define ROX_LIST_FOREACH_RETURN_VALUE(val) \
    rox_list_iter_free(list_iter_53d46d2a04458e7b);                 \
    return val

#define ROX_LIST_FOREACH_RETURN \
    rox_list_iter_free(list_iter_53d46d2a04458e7b);                 \
    return