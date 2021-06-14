#pragma once

#include "options.h"

#define ROX_DEFAULT_STORAGE_LOCATION "/tmp/rox/storage"

typedef struct RoxStorageEntry RoxStorageEntry;

typedef struct RoxStorage RoxStorage;

typedef void (*rox_storage_write_func)(void *target, RoxStorageEntry *entry, const char *data);

typedef char *(*rox_storage_read_func)(void *target, RoxStorageEntry *entry);

typedef void (*rox_storage_entry_init_func)(void *target, RoxStorageEntry *entry);

typedef void (*rox_storage_entry_uninit_func)(void *target, RoxStorageEntry *entry);

typedef void (*rox_storage_entry_delete_func)(void *target, RoxStorageEntry *entry);

typedef void (*rox_storage_target_free_func)(void *target);

typedef struct RoxStorageConfig {
    const char *location;
    void *target;
    rox_storage_target_free_func target_free;
    rox_storage_entry_init_func entry_init;
    rox_storage_entry_uninit_func entry_uninit;
    rox_storage_write_func entry_write;
    rox_storage_read_func entry_read;
    rox_storage_entry_delete_func entry_delete;
} RoxStorageConfig;

/**
 * The passed <code>config</code> data can be disposed right after setup.
 *
 * @param options Not <code>NULL</code>.
 * @param config Not <code>NULL</code>. The value must be freed by the caller.
 */
ROX_API void rox_options_set_storage_config(RoxOptions *options, RoxStorageConfig *config);

/**
 * Alternative to <code>rox_options_set_storage_config</code>.
 * Allows to only change storage location but still use the default
 * storage implementation.
 *
 * @param options Not <code>NULL</code>.
 * @param location Not <code>NULL</code>. The value should be freed by the caller after calling setup.
 */
ROX_API void rox_options_set_storage_location(RoxOptions *options, const char *location);

/**
 * @param entry Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_API const char *rox_storage_entry_get_name(RoxStorageEntry *entry);

/**
 * @param entry Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_API void *rox_storage_entry_get_meta_data(RoxStorageEntry *entry);

typedef void (*rox_storage_entry_free_meta_data_func)(void *data);

/**
 * @param entry Not <code>NULL</code>.
 * @param data May be <code>NULL</code>.
 * @param free_func May be <code>NULL</code>.
 */
ROX_API void rox_storage_entry_set_meta_data(
        RoxStorageEntry *entry,
        void *data,
        rox_storage_entry_free_meta_data_func free_func);
