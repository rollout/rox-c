#include <collectc/hashtable.h>
#include <collectc/list.h>
#include <assert.h>
#include "util.h"
#include "repositories.h"

//
// CustomPropertyRepository
//

struct ROX_INTERNAL CustomPropertyRepository {
    HashTable *custom_properties;
    custom_property_handler handler;
};

CustomPropertyRepository *custom_property_repository_create() {
    CustomPropertyRepository *repository = calloc(1, sizeof(CustomPropertyRepository));
    hashtable_new(&repository->custom_properties);
    return repository;
}

void ROX_INTERNAL custom_property_repository_add_custom_property(
        CustomPropertyRepository *repository,
        CustomProperty *property) {
    assert(repository);
    assert(property);
    char *name = custom_property_get_name(property);
    void *previous;
    if (hashtable_remove(repository->custom_properties, name, &previous) == CC_OK) {
        custom_property_free(previous);
    }
    hashtable_add(repository->custom_properties, name, property);
    if (repository->handler) {
        repository->handler(property);
    }
}

bool ROX_INTERNAL custom_property_repository_add_custom_property_if_not_exists(
        CustomPropertyRepository *repository,
        CustomProperty *property) {
    assert(repository);
    assert(property);
    char *name = custom_property_get_name(property);
    if (!hashtable_contains_key(repository->custom_properties, name)) {
        hashtable_add(repository->custom_properties, name, property);
        if (repository->handler) {
            repository->handler(property);
        }
        return true;
    }
    return false;
}

CustomProperty *ROX_INTERNAL custom_property_repository_get_custom_property(
        CustomPropertyRepository *repository,
        const char *property_name) {
    assert(repository);
    assert(property_name);
    void *ptr;
    return (hashtable_get(repository->custom_properties, (void *) property_name, &ptr) == CC_OK)
           ? ptr : NULL;
}

HashTable *ROX_INTERNAL custom_property_repository_get_all_custom_properties(CustomPropertyRepository *repository) {
    assert(repository);
    return repository->custom_properties;
}

void ROX_INTERNAL custom_property_repository_set_handler(
        CustomPropertyRepository *repository,
        custom_property_handler handler) {
    assert(repository);
    repository->handler = handler;
}

void custom_property_repository_free(CustomPropertyRepository *repository) {
    assert(repository);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, repository->custom_properties, {
        custom_property_free(entry->value);
    })
    hashtable_destroy(repository->custom_properties);
    free(repository);
}

//
// ExperimentRepository
//

struct ROX_INTERNAL ExperimentRepository {
    List *experiments;
};

ExperimentRepository *ROX_INTERNAL experiment_repository_create() {
    ExperimentRepository *repository = calloc(1, sizeof(ExperimentRepository));
    list_new(&repository->experiments);
    return repository;
}

void ROX_INTERNAL experiment_repository_set_experiments(
        ExperimentRepository *repository,
        List *experiments) {
    assert(repository);
    assert(experiments);
    list_destroy_cb(repository->experiments, (void (*)(void *)) &experiment_model_free);
    repository->experiments = experiments;
}

ExperimentModel *ROX_INTERNAL experiment_repository_get_experiment_by_flag(
        ExperimentRepository *repository,
        const char *flag_name) {
    assert(repository);
    assert(flag_name);
    LIST_FOREACH(model, repository->experiments, {
        if (str_in_list(flag_name, ((ExperimentModel *) model)->flags)) {
            return model;
        }
    })
    return NULL;
}

List *ROX_INTERNAL experiment_repository_get_all_experiments(ExperimentRepository *repository) {
    assert(repository);
    return repository->experiments;
}

void ROX_INTERNAL experiment_repository_free(ExperimentRepository *repository) {
    assert(repository);
    list_destroy_cb(repository->experiments, (void (*)(void *)) &experiment_model_free);
    free(repository);
}

//
// FlagRepository
//

struct ROX_INTERNAL FlagRepository {
    HashTable *variants;
    flag_added_callback callback;
};

FlagRepository *ROX_INTERNAL flag_repository_create() {
    FlagRepository *repository = calloc(1, sizeof(FlagRepository));
    hashtable_new(&repository->variants);
    return repository;
};

void ROX_INTERNAL flag_repository_add_flag(
        FlagRepository *repository,
        Variant *variant,
        const char *name) {
    assert(repository);
    assert(variant);
    assert(name);
    assert(!str_is_empty(name));
    char *variant_name = variant_get_name(variant);
    if (str_is_empty(variant_name)) {
        variant_set_name(variant, name);
    }
    void *key = (void *) name;
    hashtable_add(repository->variants, key, variant);
    if (repository->callback) {
        repository->callback(variant);
    }
}

Variant *ROX_INTERNAL flag_repository_get_flag(
        FlagRepository *repository,
        const char *name) {
    assert(repository);
    assert(name);
    assert(!str_is_empty(name));
    void *variant;
    if (hashtable_get(repository->variants, (void *) name, &variant) == CC_OK) {
        return variant;
    }
    return NULL;
}

HashTable *ROX_INTERNAL flag_repository_get_all_flags(FlagRepository *repository) {
    assert(repository);
    return repository->variants;
}

void ROX_INTERNAL flag_repository_add_flag_added_callback(
        FlagRepository *repository,
        flag_added_callback callback) {
    assert(repository);
    assert(callback);
    repository->callback = callback;
}

void ROX_INTERNAL flag_repository_free(FlagRepository *repository) {
    assert(repository);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, repository->variants, {
        variant_free(entry->value);
    })
    hashtable_destroy(repository->variants);
    free(repository);
}
