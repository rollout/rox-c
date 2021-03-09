#pragma once

#include "options.h"

#define ROX_DEFAULT_STORAGE_LOCATION "/tmp/rox/storage"

typedef struct StorageEntry StorageEntry;

typedef struct Storage Storage;

typedef void (*storage_write_func)(StorageEntry *entry, const char *data);

typedef char *(*storage_read_func)(StorageEntry *entry);

typedef void (*storage_entry_init_func)(StorageEntry *entry);

typedef void (*storage_entry_uninit_func)(StorageEntry *entry);

typedef void (*storage_entry_delete_func)(StorageEntry *entry);

typedef struct StorageConfig {
    const char *location;
    storage_entry_init_func entry_init;
    storage_entry_uninit_func entry_uninit;
    storage_write_func entry_write;
    storage_read_func entry_read;
    storage_entry_delete_func entry_delete;
} StorageConfig;

/**
 * The passed <code>config</code> data can be disposed right after setup.
 *
 * @param options Not <code>NULL</code>.
 * @param config Not <code>NULL</code>. The value must be freed by the caller.
 */
ROX_API void rox_options_set_storage_config(RoxOptions *options, StorageConfig *config);

/**
 * @param entry Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API const char *storage_entry_get_name(StorageEntry *entry);

/**
 * @param entry Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_API void *storage_entry_get_meta_data(StorageEntry *entry);

typedef void (*storage_entry_free_meta_data_func)(StorageEntry *entry, void *data);

/**
 * @param entry Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 * @param free_func May be <code>NULL</code>.
 */
ROX_API void storage_entry_set_meta_data(
        StorageEntry *entry,
        void *data,
        storage_entry_free_meta_data_func free_func);
