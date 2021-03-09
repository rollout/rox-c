#pragma once

#include "rox/defs.h"
#include "rox/collections.h"
#include "rox/storage.h"

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL StorageConfig *get_storage_config_from_options(RoxOptions *options);

/**
 * @param config May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL Storage *storage_create(StorageConfig *config);

/**
 * @param storage Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL StorageEntry *storage_get_entry(Storage *storage, const char *name);

/**
 * The caller is responsible for freeing all the resources.
 *
 * @param entry Not <code>NULL</code>.
 * @param values Not <code>NULL</code>. Map with string keys and values.
 */
ROX_INTERNAL void storage_write_string_key_value_map(StorageEntry *entry, RoxMap *values);

/**
 * The returned value must be freed by the caller.
 *
 * @return May be <code>NULL</code>. Map with string keys and values.
 */
ROX_INTERNAL RoxMap *storage_read_string_key_value_map(StorageEntry *entry);

/**
 * @param storage Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_INTERNAL void storage_delete_entry(StorageEntry *entry);

/**
 * @param storage Not <code>NULL</code>.
 */
ROX_INTERNAL void storage_free(Storage *storage);
