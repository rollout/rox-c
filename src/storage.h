#pragma once

#include "rox/defs.h"
#include "rox/collections.h"
#include "rox/storage.h"
#include "client.h"

/**
 * @param storage Not <code>NULL</code>.
 * @param settings Not <code>NULL</code>.
 */
ROX_INTERNAL void storage_init(RoxStorage *storage, SdkSettings *settings);

/**
 * @param settings Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL RoxStorage *storage_get_from_settings(SdkSettings *settings);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL RoxStorageConfig *get_storage_config_from_options(RoxOptions *options);

/**
 * @param options Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL char *get_storage_location_from_options(RoxOptions *options);

/**
 * @param config May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStorage *storage_create(RoxStorageConfig *config);

/**
 * @param location Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStorage *storage_create_with_location(const char *location);

/**
 * @param options Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStorage *storage_create_from_options(RoxOptions *options);

/**
 * @param storage Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxStorageEntry *storage_get_entry(RoxStorage *storage, const char *name);

/**
 * The caller is responsible for freeing all the resources.
 *
 * @param entry Not <code>NULL</code>.
 * @param values Not <code>NULL</code>. Map with string keys and values.
 */
ROX_INTERNAL void storage_write_string_key_value_map(RoxStorageEntry *entry, RoxMap *values);

/**
 * The returned value must be freed by the caller including its keys and values.
 *
 * @return May be <code>NULL</code>. Map with string keys and values.
 */
ROX_INTERNAL RoxMap *storage_read_string_key_value_map(RoxStorageEntry *entry);

/**
 * @param storage Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_INTERNAL void storage_delete_entry(RoxStorageEntry *entry);

/**
 * @param storage Not <code>NULL</code>.
 */
ROX_INTERNAL void storage_free(RoxStorage *storage);
