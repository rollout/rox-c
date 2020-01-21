#pragma once

#include <collectc/hashtable.h>
#include "roxapi.h"

typedef struct ROX_INTERNAL Context Context;

void *ROX_INTERNAL context_get(Context *context, const char *key);

Context *ROX_INTERNAL context_create_from_hashtable(HashTable *map);

Context *ROX_INTERNAL context_create_merged(Context *global_context, Context *local_context);

void ROX_INTERNAL context_free(Context *context);
