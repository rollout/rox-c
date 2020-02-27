#include <assert.h>
#include "context.h"
#include "util.h"

struct RoxContext {
    HashTable *map;
    void *target;
    rox_context_get_value_func get_value;
    rox_context_free_target_func fee_target;
};

ROX_API RoxDynamicValue *rox_context_get(RoxContext *context, const char *key) {
    assert(context);
    assert(key);
    if (context->get_value) {
        return context->get_value(context->target, key);
    }
    void *ptr;
    if (hashtable_get(context->map, (void *) key, &ptr) == CC_OK) {
        return ptr;
    }
    return NULL;
}

ROX_API RoxContext *rox_context_create_empty() {
    RoxContext *context = calloc(1, sizeof(RoxContext));
    hashtable_new(&context->map);
    return context;
}

ROX_API RoxContext *rox_context_create_from_map(HashTable *map) {
    assert(map);
    RoxContext *context = calloc(1, sizeof(RoxContext));
    context->map = map;
    return context;
}

typedef struct MergedContext {
    RoxContext *global_context;
    RoxContext *local_context;
} MergedContext;

static RoxDynamicValue *_merged_context_get_value(void *target, const char *key) {
    assert(target);
    assert(key);
    MergedContext *merged = (MergedContext *) target;
    if (merged->local_context) {
        RoxDynamicValue *value = rox_context_get(merged->local_context, key);
        if (value) {
            return value;
        }
    }
    if (merged->global_context) {
        RoxDynamicValue *value = rox_context_get(merged->global_context, key);
        if (value) {
            return value;
        }
    }
    return NULL;
}

ROX_API RoxContext *rox_context_create_merged(RoxContext *global_context, RoxContext *local_context) {
    MergedContext *merged = calloc(1, sizeof(MergedContext));
    merged->global_context = global_context;
    merged->local_context = local_context;
    RoxContextConfig config = {merged, &_merged_context_get_value, (rox_context_free_target_func) &free};
    return rox_context_create_custom(&config);
}

ROX_API RoxContext *rox_context_create_custom(RoxContextConfig *config) {
    assert(config);
    RoxContext *context = calloc(1, sizeof(RoxContext));
    context->target = config->target;
    context->get_value = config->get_value_func;
    context->fee_target = config->fee_target_func;
    return context;
}

ROX_API void rox_context_free(RoxContext *context) {
    assert(context);
    if (context->map) {
        rox_hash_table_free_with_keys_and_values_cb(
                context->map,
                &free,
                (void (*)(void *)) &rox_dynamic_value_free);
    }
    if (context->target && context->fee_target) {
        context->fee_target(context->target);
    }
    free(context);
}
