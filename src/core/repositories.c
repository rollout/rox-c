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
    void *target;
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
        repository->handler(repository->target, property);
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
            repository->handler(repository->target, property);
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
        void *target,
        custom_property_handler handler) {
    assert(repository);
    assert(handler);
    repository->target = target;
    repository->handler = handler;
}

void custom_property_repository_free(CustomPropertyRepository *repository) {
    assert(repository);
    rox_map_free_with_values_cb(repository->custom_properties,
                                (void (*)(void *)) &custom_property_free);
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
    ListIter i;
    list_iter_init(&i, repository->experiments);
    ExperimentModel *model;
    while (list_iter_next(&i, (void **) &model) != CC_ITER_END) {
        List *flags = model->flags;
        if (flags && str_in_list(flag_name, flags)) {
            return model;
        }
    }
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

typedef struct ROX_INTERNAL FlagAddedCallback {
    void *target;
    flag_added_callback callback;
} FlagAddedCallback;

struct ROX_INTERNAL FlagRepository {
    HashTable *variants;
    List *callbacks;
};

FlagRepository *ROX_INTERNAL flag_repository_create() {
    FlagRepository *repository = calloc(1, sizeof(FlagRepository));
    hashtable_new(&repository->variants);
    list_new(&repository->callbacks);
    return repository;
};

void ROX_INTERNAL flag_repository_add_flag(
        FlagRepository *repository,
        RoxVariant *variant,
        const char *name) {
    assert(repository);
    assert(variant);
    assert(name);
    assert(!str_is_empty(name));
    const char *variant_name = variant_get_name(variant);
    if (str_is_empty(variant_name)) {
        variant_set_name(variant, name);
    }
    void *key = (void *) name;
    hashtable_add(repository->variants, key, variant);
    LIST_FOREACH(item, repository->callbacks, {
        FlagAddedCallback *callback = (FlagAddedCallback *) item;
        callback->callback(callback->target, variant);
    })
}

RoxVariant *ROX_INTERNAL flag_repository_get_flag(
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
        void *target,
        flag_added_callback callback) {
    assert(repository);
    assert(callback);
    FlagAddedCallback *item = calloc(1, sizeof(FlagAddedCallback));
    item->target = target;
    item->callback = callback;
    list_add(repository->callbacks, item);
}

void ROX_INTERNAL flag_repository_free(FlagRepository *repository) {
    assert(repository);
    list_destroy_cb(repository->callbacks, &free);
    rox_map_free_with_values_cb(repository->variants,
                                (void (*)(void *)) &variant_free);
    free(repository);
}

//
// TargetGroupRepository
//

struct ROX_INTERNAL TargetGroupRepository {
    List *target_groups;
};

TargetGroupRepository *ROX_INTERNAL target_group_repository_create() {
    TargetGroupRepository *repository = calloc(1, sizeof(TargetGroupRepository));
    list_new(&repository->target_groups);
    return repository;
}

void ROX_INTERNAL target_group_repository_set_target_groups(
        TargetGroupRepository *repository,
        List *target_groups) {
    assert(repository);
    assert(target_groups);
    list_destroy_cb(repository->target_groups, (void (*)(void *)) &target_group_model_free);
    repository->target_groups = target_groups;
}

TargetGroupModel *ROX_INTERNAL target_group_repository_get_target_group(
        TargetGroupRepository *repository,
        const char *id) {
    assert(repository);
    assert(id);
    TargetGroupModel *model = NULL;
    ListIter i;
    list_iter_init(&i, repository->target_groups);
    TargetGroupModel *m;
    while (list_iter_next(&i, (void **) &m) != CC_ITER_END) {
        if (str_equals(m->id, id)) {
            model = m;
            break;
        }
    }
    return model;
}

void ROX_INTERNAL target_group_repository_free(TargetGroupRepository *repository) {
    assert(repository);
    list_destroy_cb(repository->target_groups, (void (*)(void *)) &target_group_model_free);
    free(repository);
}
