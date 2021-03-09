#include <assert.h>
#include <stdlib.h>
#include "storage.h"
#include "core/options.h"
#include "core/logging.h"
#include "collections.h"
#include "util.h"

struct StorageEntry {
    char *name;
    Storage *storage;
    storage_write_func entry_write;
    storage_read_func entry_read;
    storage_entry_uninit_func entry_uninit;
    void *meta;
    storage_entry_free_meta_data_func free_meta_func;
};

struct Storage {
    RoxMap *entries;
    const char *location;
    storage_entry_init_func entry_init;
    storage_entry_uninit_func entry_uninit;
    storage_write_func entry_write;
    storage_read_func entry_read;
    storage_entry_delete_func entry_delete;
};

static const char *STORAGE_OPTIONS_KEY = "storage";

static void storage_entry_free_meta(StorageEntry *entry) {
    if (entry->meta) {
        if (entry->free_meta_func) {
            entry->free_meta_func(entry, entry->meta);
        }
    }
}

ROX_API void rox_options_set_storage_config(RoxOptions *options, StorageConfig *config) {
    assert(options);
    rox_options_set_extra(options, STORAGE_OPTIONS_KEY, config, NULL);
}

ROX_API const char *storage_entry_get_name(StorageEntry *entry) {
    assert(entry);
    return entry->name;
}

ROX_API void *storage_entry_get_meta_data(StorageEntry *entry) {
    assert(entry);
    return entry->meta;
}

ROX_API void storage_entry_set_meta_data(StorageEntry *entry, void *data, storage_entry_free_meta_data_func free_func) {
    assert(entry);
    storage_entry_free_meta(entry);
    entry->meta = data;
    entry->free_meta_func = free_func;
}

ROX_INTERNAL StorageConfig *get_storage_config_from_options(RoxOptions *options) {
    assert(options);
    return rox_options_get_extra(options, STORAGE_OPTIONS_KEY);
}

static void default_storage_free_meta_func(StorageEntry *entry, void *meta) {
    assert(entry);
    assert(meta);
    free(meta);
}

static void default_storage_entry_init_func(StorageEntry *entry) {
    assert(entry);
    if (!mkdirs(entry->storage->location)) {
        ROX_WARN("Failed to create storage location %s", entry->storage->location);
        return;
    }
    char *path = mem_str_format("%s/%s.json", entry->storage->location, entry->name);
    storage_entry_set_meta_data(entry, path, default_storage_free_meta_func);
}

static void default_storage_write_func(StorageEntry *entry, const char *data) {
    assert(entry);
    assert(data);
    char *path = storage_entry_get_meta_data(entry);
    if (!path) {
        ROX_WARN("Storage entry %s is not initialized with file location", entry->name);
        return;
    }
    if (!str_to_file(path, data)) {
        ROX_WARN("Failed to write data to storage entry %s, file location: %s", entry->name, path);
        return;
    }
}

static char *default_storage_read_func(StorageEntry *entry) {
    assert(entry);
    char *path = storage_entry_get_meta_data(entry);
    if (!path) {
        ROX_WARN("Storage entry %s is not initialized with file location", entry->name);
        return NULL;
    }
    return mem_file_read(path);
}

static void default_storage_entry_uninit_func(StorageEntry *entry) {
    assert(entry);
    // Stub
}

typedef void (*storage_entry_delete_func)(StorageEntry *entry);

static StorageConfig default_storage_config = {
        ROX_DEFAULT_STORAGE_LOCATION,
        default_storage_entry_init_func,
        default_storage_entry_uninit_func,
        default_storage_write_func,
        default_storage_read_func
};

ROX_INTERNAL Storage *storage_create(StorageConfig *config) {
    if (!config) {
        config = &default_storage_config;
    }
    Storage *storage = calloc(1, sizeof(Storage));
    storage->location = config->location;
    storage->entry_init = config->entry_init;
    storage->entry_uninit = config->entry_uninit;
    storage->entry_read = config->entry_read;
    storage->entry_write = config->entry_write;
    storage->entry_delete = config->entry_delete;
    storage->entries = ROX_EMPTY_MAP;
    return storage;
}

ROX_INTERNAL StorageEntry *storage_get_entry(Storage *storage, const char *name) {
    assert(storage);
    assert(name);
    void *data;
    if (rox_map_get(storage->entries, (void *) name, &data)) {
        return data;
    }
    StorageEntry *entry = calloc(1, sizeof(StorageEntry));
    entry->name = mem_copy_str(name);
    entry->storage = storage;
    entry->entry_write = storage->entry_write;
    entry->entry_read = storage->entry_read;
    entry->entry_uninit = storage->entry_uninit;
    rox_map_add(storage->entries, mem_copy_str(name), entry);
    if (storage->entry_init) {
        storage->entry_init(entry);
    }
    return entry;
}

ROX_INTERNAL void storage_write_string_key_value_map(StorageEntry *entry, RoxMap *values) {
    assert(entry);
    assert(values);

    cJSON *json = cJSON_CreateObject();
    ROX_MAP_FOREACH(key, value, values, {
        cJSON *item = cJSON_CreateString(value);
        cJSON_AddItemToObject(json, key, item);
    })

    char *data = ROX_JSON_SERIALIZE(json);
    cJSON_Delete(json);

    entry->entry_write(entry, data);
    free(data);
}

ROX_INTERNAL RoxMap *storage_read_string_key_value_map(StorageEntry *entry) {
    assert(entry);

    char *data = entry->entry_read(entry);
    if (!data) {
        ROX_DEBUG("No data read for entry %s", entry->name);
        return NULL;
    }

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        ROX_WARN("Failed to deserialize JSON data %s for entry %s", data, entry->name);
        free(data);
        return NULL;
    }

    RoxMap *values = ROX_EMPTY_MAP;
    cJSON *child = json->child;
    while (child) {
        char *value = cJSON_GetStringValue(child);
        rox_map_add(values, mem_copy_str(child->string), mem_copy_str(value));
        child = child->next;
    }
    cJSON_Delete(json);
    free(data);

    return values;
}

static void storage_close_entry(StorageEntry *entry) {
    assert(entry);
    if (entry->entry_uninit) {
        entry->entry_uninit(entry);
    }
    storage_entry_free_meta(entry);
    free(entry->name);
    free(entry);
}

static void storage_entry_free(void *data) {
    StorageEntry *entry = data;
    storage_close_entry(entry);
}

ROX_INTERNAL void storage_delete_entry(StorageEntry *entry) {
    assert(entry);
    void *data;
    if (rox_map_remove(entry->storage->entries, (void *) entry->name, &data)) {
        entry->storage->entry_delete(entry);
        storage_close_entry(entry);
    }
}

ROX_INTERNAL void storage_free(Storage *storage) {
    assert(storage);
    rox_map_free_with_keys_and_values_cb(storage->entries, free, storage_entry_free);
    free(storage);
}
