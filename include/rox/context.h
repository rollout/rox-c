#pragma once

#include <rox/defs.h>
#include <rox/collections.h>
#include <rox/values.h>

typedef struct RoxContext RoxContext;

typedef RoxDynamicValue *(*rox_context_get_value_func)(void *target, const char *key);

typedef void *(*rox_context_free_target_func)(void *target);

/**
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_empty();

/**
 * Creates context from the given hashtable. The ownership on the given hash table,
 * including its keys and values, is delegated to the created context,
 * and all of the memory will be freed in <code>rox_context_free()</code>.
 *
 * @param map Not <code>NULL</code>. Keys are strings, values are <code>RoxDynamicValue *</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_from_map(RoxMap *map);

/**
 * The called holds the ownership on the given contexts. They will <em>NOT</em> be freed when
 * the returned <code>context</code> is destroyed.
 *
 * @param global_context May be <code>NULL</code>.
 * @param local_context May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_merged(RoxContext *global_context, RoxContext *local_context);

typedef struct RoxContextConfig {
    void *target;
    rox_context_get_value_func get_value_func;
    rox_context_free_target_func fee_target_func;
} RoxContextConfig;

/**
 * @param config Not <code>NULL</code>.
 */
ROX_API RoxContext *rox_context_create_custom(RoxContextConfig *config);

/**
 * @param context Not <code>NULL</code>.
 */
ROX_API void rox_context_free(RoxContext *context);

/**
 * @param context Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>. If returned, the value must be freed by the caller via invoking <code>rox_dynamic_value_free</code>.
 */
ROX_API RoxDynamicValue *rox_context_get(RoxContext *context, const char *key);
