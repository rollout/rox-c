#include <assert.h>
#include "context.h"

struct ROX_INTERNAL Context {
    HashTable *map;
    bool key_value_ownership_delegated;
};

void *ROX_INTERNAL context_get(Context *context, const char *key) {
    assert(context);
    assert(key);
    void *ptr;
    if (hashtable_get(context->map, (void *) key, &ptr) == CC_OK) {
        return ptr;
    }
    return NULL;
}

void ROX_INTERNAL context_copy_data(Context *context, HashTable *map) {
    assert(context);
    assert(map);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, map, {
        hashtable_add(context->map, entry->key, entry->value);
    })
}

Context *ROX_INTERNAL context_create_empty() {
    Context *context = calloc(1, sizeof(Context));
    hashtable_new(&context->map);
    return context;
}

Context *ROX_INTERNAL context_create_from_hashtable(HashTable *map) {
    assert(map);
    Context *context = calloc(1, sizeof(Context));
    context->map = map;
    return context;
}

Context *ROX_INTERNAL context_create_merged(Context *global_context, Context *local_context) {
    Context *context = context_create_empty();
    if (global_context) {
        context_copy_data(context, global_context->map);
        global_context->key_value_ownership_delegated = true;
    }
    if (local_context) {
        context_copy_data(context, local_context->map);
        local_context->key_value_ownership_delegated = true;
    }
    return context;
}

void ROX_INTERNAL context_free(Context *context) {
    assert(context);
    if (!context->key_value_ownership_delegated) {
        TableEntry *entry;
        HASHTABLE_FOREACH(entry, context->map, {
            free(entry->key);
            free(entry->value);
        });
    }
    hashtable_destroy(context->map);
    free(context);
}