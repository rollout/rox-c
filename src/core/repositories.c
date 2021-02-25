#include <assert.h>
#include "util.h"
#include "repositories.h"
#include "collections.h"

//
// CustomPropertyRepository
//

struct CustomPropertyRepository {
    RoxMap *custom_properties;
    void *target;
    custom_property_handler handler;
};

CustomPropertyRepository *custom_property_repository_create() {
    CustomPropertyRepository *repository = calloc(1, sizeof(CustomPropertyRepository));
    repository->custom_properties = rox_map_create();
    return repository;
}

ROX_INTERNAL void custom_property_repository_add_custom_property(
        CustomPropertyRepository *repository,
        CustomProperty *property) {
    assert(repository);
    assert(property);
    char *name = custom_property_get_name(property);
    void *previous;
    if (rox_map_remove(repository->custom_properties, name, &previous)) {
        custom_property_free(previous);
    }
    rox_map_add(repository->custom_properties, name, property);
    if (repository->handler) {
        repository->handler(repository->target, property);
    }
}

ROX_INTERNAL bool custom_property_repository_add_custom_property_if_not_exists(
        CustomPropertyRepository *repository,
        CustomProperty *property) {
    assert(repository);
    assert(property);
    char *name = custom_property_get_name(property);
    if (!rox_map_contains_key(repository->custom_properties, name)) {
        rox_map_add(repository->custom_properties, name, property);
        if (repository->handler) {
            repository->handler(repository->target, property);
        }
        return true;
    }
    return false;
}

ROX_INTERNAL CustomProperty *custom_property_repository_get_custom_property(
        CustomPropertyRepository *repository,
        const char *property_name) {
    assert(repository);
    assert(property_name);
    void *ptr;
    return (rox_map_get(repository->custom_properties, (void *) property_name, &ptr)) ? ptr : NULL;
}

ROX_INTERNAL RoxMap *custom_property_repository_get_all_custom_properties(CustomPropertyRepository *repository) {
    assert(repository);
    return repository->custom_properties;
}

ROX_INTERNAL void custom_property_repository_set_handler(
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

struct ExperimentRepository {
    RoxList *experiments;
    RoxList *previous_experiments;
};

ROX_INTERNAL ExperimentRepository *experiment_repository_create() {
    ExperimentRepository *repository = calloc(1, sizeof(ExperimentRepository));
    repository->experiments = rox_list_create();
    return repository;
}

ROX_INTERNAL void experiment_repository_set_experiments(
        ExperimentRepository *repository,
        RoxList *experiments) {
    assert(repository);
    assert(experiments);
    // Don't free experiments immediately since they still could be referenced in other threads during the configuration fetch
    if (repository->previous_experiments) {
        rox_list_free_cb(repository->previous_experiments, (void (*)(void *)) &experiment_model_free);
    }
    repository->previous_experiments = repository->experiments;
    repository->experiments = experiments;
}

ROX_INTERNAL ExperimentModel *experiment_repository_get_experiment_by_flag(
        ExperimentRepository *repository,
        const char *flag_name) {
    assert(repository);
    assert(flag_name);
    ROX_LIST_FOREACH(item, repository->experiments, {
        ExperimentModel *model = (ExperimentModel *) item;
        RoxList *flags = model->flags;
        if (flags && str_in_list(flag_name, flags)) {
            ROX_LIST_FOREACH_RETURN_VALUE(model);
        }
    })
    return NULL;
}

ROX_INTERNAL RoxList *experiment_repository_get_all_experiments(ExperimentRepository *repository) {
    assert(repository);
    return repository->experiments;
}

ROX_INTERNAL void experiment_repository_free(ExperimentRepository *repository) {
    assert(repository);
    if (repository->previous_experiments) {
        rox_list_free_cb(repository->previous_experiments, (void (*)(void *)) &experiment_model_free);
    }
    rox_list_free_cb(repository->experiments, (void (*)(void *)) &experiment_model_free);
    free(repository);
}

//
// FlagRepository
//

typedef struct FlagAddedCallback {
    void *target;
    flag_added_callback callback;
} FlagAddedCallback;

struct FlagRepository {
    RoxMap *variants;
    RoxList *callbacks;
};

ROX_INTERNAL FlagRepository *flag_repository_create() {
    FlagRepository *repository = calloc(1, sizeof(FlagRepository));
    repository->variants = rox_map_create();
    repository->callbacks = rox_list_create();
    return repository;
};

ROX_INTERNAL void flag_repository_add_flag(
        FlagRepository *repository,
        RoxStringBase *variant,
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
    rox_map_add(repository->variants, key, variant);
    ROX_LIST_FOREACH(item, repository->callbacks, {
        FlagAddedCallback *callback = (FlagAddedCallback *) item;
        callback->callback(callback->target, variant);
    })
}

ROX_INTERNAL RoxStringBase *flag_repository_get_flag(
        FlagRepository *repository,
        const char *name) {
    assert(repository);
    assert(name);
    assert(!str_is_empty(name));
    void *variant;
    if (rox_map_get(repository->variants, (void *) name, &variant)) {
        return variant;
    }
    return NULL;
}

ROX_INTERNAL RoxMap *flag_repository_get_all_flags(FlagRepository *repository) {
    assert(repository);
    return repository->variants;
}

ROX_INTERNAL void flag_repository_add_flag_added_callback(
        FlagRepository *repository,
        void *target,
        flag_added_callback callback) {
    assert(repository);
    assert(callback);
    FlagAddedCallback *item = calloc(1, sizeof(FlagAddedCallback));
    item->target = target;
    item->callback = callback;
    rox_list_add(repository->callbacks, item);
}

ROX_INTERNAL void flag_repository_free(FlagRepository *repository) {
    assert(repository);
    rox_list_free_cb(repository->callbacks, &free);
    rox_map_free_with_values_cb(repository->variants,
                                (void (*)(void *)) &variant_free);
    free(repository);
}

//
// TargetGroupRepository
//

struct TargetGroupRepository {
    RoxList *target_groups;
    RoxList *previous_target_groups;
};

ROX_INTERNAL TargetGroupRepository *target_group_repository_create() {
    TargetGroupRepository *repository = calloc(1, sizeof(TargetGroupRepository));
    repository->target_groups = rox_list_create();
    return repository;
}

ROX_INTERNAL void target_group_repository_set_target_groups(
        TargetGroupRepository *repository,
        RoxList *target_groups) {
    assert(repository);
    assert(target_groups);
    // Don't free target groups immediately since they still could be referenced in other threads during the configuration fetch
    if (repository->previous_target_groups) {
        rox_list_free_cb(repository->previous_target_groups, (void (*)(void *)) &target_group_model_free);
    }
    repository->previous_target_groups = repository->target_groups;
    repository->target_groups = target_groups;
}

ROX_INTERNAL TargetGroupModel *target_group_repository_get_target_group(
        TargetGroupRepository *repository,
        const char *id) {
    assert(repository);
    assert(id);
    TargetGroupModel *model = NULL;
    ROX_LIST_FOREACH(item, repository->target_groups, {
        TargetGroupModel *m = (TargetGroupModel *) item;
        if (str_equals(m->id, id)) {
            model = m;
            break;
        }
    })
    return model;
}

ROX_INTERNAL void target_group_repository_free(TargetGroupRepository *repository) {
    assert(repository);
    if (repository->previous_target_groups) {
        rox_list_free_cb(repository->previous_target_groups, (void (*)(void *)) &target_group_model_free);
    }
    rox_list_free_cb(repository->target_groups, (void (*)(void *)) &target_group_model_free);
    free(repository);
}
