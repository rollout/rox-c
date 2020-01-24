#pragma once

#include <collectc/hashtable.h>
#include "roxapi.h"

typedef struct ROX_INTERNAL Context Context;

/**
 * Creates context from the given hashtable. The ownership of keys and values of the given hash table
 * is delegated to the created context, and will be freed in <code>context_free()</code>.
 * Thus the caller must not free them.
 *
 * @param map Not <code>NULL</code>. Keys are strings, value can be arbitrary objects.
 * @return Not <code>NULL</code>.
 */
Context *ROX_INTERNAL context_create_from_hashtable(HashTable *map);

/**
 * The ownership of the given contexts' keys and values are delegated to the created context,
 * but the caller is still needed to call <code>context_free()</code> on both of them,
 * which would free all the rest of the memory used.
 *
 * @param global_context May be <code>NULL</code>.
 * @param local_context May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
Context *ROX_INTERNAL context_create_merged(Context *global_context, Context *local_context);

/**
 * @param context Not <code>NULL</code>.
 * @param key Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
void *ROX_INTERNAL context_get(Context *context, const char *key);

/**
 * @param context Not <code>NULL</code>.
 */
void ROX_INTERNAL context_free(Context *context);
