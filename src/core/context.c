#include <assert.h>
#include "context.h"

struct ROX_INTERNAL Context {
    HashTable *map;
};

void *ROX_INTERNAL context_get(Context *context, const char *key) {
    assert(context);
    assert(key);
    void *ptr;
    if (hashtable_get(context->map, (void *) key, &ptr) == 0) {
        return ptr;
    }
    return NULL;
}

Context *ROX_INTERNAL context_create_empty() {
    Context *context = calloc(1, sizeof(Context));
    hashtable_new(&context->map);
    return context;
}

void ROX_INTERNAL context_copy_data(Context *context, HashTable *map) {
    assert(context);
    assert(map);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        hashtable_add(context->map, entry->key, entry->value);
    })
}

Context *ROX_INTERNAL context_create_from_hashtable(HashTable *map) {
    assert(map);
    Context *context = context_create_empty();
    context_copy_data(context, map);
    return context;
}

Context *ROX_INTERNAL context_create_merged(Context *global_context, Context *local_context) {
    assert(global_context);
    assert(local_context);
    Context *context = context_create_empty();
    if (global_context) {
        context_copy_data(context, global_context->map);
    }
    if (local_context) {
        context_copy_data(context, local_context->map);
    }
    return context;
}

void ROX_INTERNAL context_free(Context *context) {
    assert(context);
    hashtable_destroy(context->map);
    free(context);
}