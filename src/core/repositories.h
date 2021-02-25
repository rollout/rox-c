#pragma once

#include "configuration/models.h"
#include "properties.h"
#include "entities.h"

//
// CustomPropertyRepository
//

typedef struct CustomPropertyRepository CustomPropertyRepository;

typedef void (*custom_property_handler)(void *target, CustomProperty *property);

/**
 * The returned object myst be freed after use by calling <code>custom_property_repository_free()</code>.
 * @return Not <code>NULL</code>.
 */
CustomPropertyRepository *custom_property_repository_create();

/**
 * Property ownership is delegated to the repository. It would be freed when
 * calling <code>custom_property_repository_free()</code>.
 *
 * @param repository Not <code>NULL</code>.
 * @param property Not <code>NULL</code>.
 */
ROX_INTERNAL void custom_property_repository_add_custom_property(
        CustomPropertyRepository *repository,
        CustomProperty *property);

/**
 * Property ownership is delegated to the repository. If added, it would be freed when
 * calling <code>custom_property_repository_free()</code>. Otherwise the caller is responsible
 * for calling <code>custom_property_free()</code> on it.
 *
 * @param repository
 * @param property
 * @return Whether the property has been added to the repo.
 */
ROX_INTERNAL bool custom_property_repository_add_custom_property_if_not_exists(
        CustomPropertyRepository *repository,
        CustomProperty *property);

ROX_INTERNAL CustomProperty *custom_property_repository_get_custom_property(
        CustomPropertyRepository *repository,
        const char *property_name);

/**
 * The returned object is maintained by the repository, you must not call <code>rox_map_destroy</code> on it.
 * @param repository Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxMap *custom_property_repository_get_all_custom_properties(
        CustomPropertyRepository *repository);

/**
 * @param repository Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_INTERNAL void custom_property_repository_set_handler(
        CustomPropertyRepository *repository,
        void *target,
        custom_property_handler handler);

/**
 * @param repository Not <code>NULL</code>.
 */
void custom_property_repository_free(CustomPropertyRepository *repository);

//
// ExperimentRepository
//

typedef struct ExperimentRepository ExperimentRepository;

/**
 * The returned object must be destroyed after use by calling <code>experiment_repository_free()</code>.
 */
ROX_INTERNAL ExperimentRepository *experiment_repository_create();

/**
 * @param repository Not <code>NULL</code>.
 * @param experiments List of <code>ExperimentModel *</code>. Not <code>NULL</code>. The ownership is delegated to repository.
 */
ROX_INTERNAL void experiment_repository_set_experiments(
        ExperimentRepository *repository,
        RoxList *experiments);

/**
 * @param repository Not <code>NULL</code>.
 * @param flag_name Not <code>NULL</code>.
 * @return RoxExperiment model or NULL if not found.
 */
ROX_INTERNAL ExperimentModel *experiment_repository_get_experiment_by_flag(
        ExperimentRepository *repository,
        const char *flag_name);

/**
 * The returned object is maintained by the repository, you must not call <code>list_destroy</code> on it.
 * @param repository Not <code>NULL</code>.
 * @return List of <code>ExperimentModel *</code>
 */
ROX_INTERNAL RoxList *experiment_repository_get_all_experiments(ExperimentRepository *repository);

/**
 * @param repository Not <code>NULL</code>.
 */
ROX_INTERNAL void experiment_repository_free(ExperimentRepository *repository);

//
// FlagRepository
//

typedef struct FlagRepository FlagRepository;

/**
 * The returned object must be freed after use by calling <code>flag_repository_free</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL FlagRepository *flag_repository_create();

/**
 * @param repository Not <code>NULL</code>.
 * @param variant Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_repository_add_flag(
        FlagRepository *repository,
        RoxStringBase *variant,
        const char *name);

/**
 * @param repository Not <code>NULL</code>.
 * @param name Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
ROX_INTERNAL RoxStringBase *flag_repository_get_flag(
        FlagRepository *repository,
        const char *name);

/**
 * The returned object is maintained by the repository, you must not call <code>rox_map_destroy</code> on it.
 * @param repository Not <code>NULL</code>.
 * @return Hash table with flag names as keys and <code>RoxStringBase *</code> as values. Not <code>NULL</code>.
 */
ROX_INTERNAL RoxMap *flag_repository_get_all_flags(FlagRepository *repository);

typedef void (*flag_added_callback)(void *target, RoxStringBase *variant);

/**
 * @param repository Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param callback Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_repository_add_flag_added_callback(
        FlagRepository *repository,
        void *target,
        flag_added_callback callback);

/**
 * @param repository Not <code>NULL</code>.
 */
ROX_INTERNAL void flag_repository_free(FlagRepository *repository);

//
// TargetGroupRepository
//

typedef struct TargetGroupRepository TargetGroupRepository;

/**
 * The returned object must be freed after use by calling <code>target_group_repository_free()</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL TargetGroupRepository *target_group_repository_create();

/**
 * By calling this method the ownership of the given <code>target_groups</code>
 * list is delegated to the given <code>repository</code>.
 *
 * @param repository Not <code>NULL</code>.
 * @param target_groups List of <code>TargetGroupModel *</code>. Not <code>NULL</code>.
 */
ROX_INTERNAL void target_group_repository_set_target_groups(
        TargetGroupRepository *repository,
        RoxList *target_groups);

/**
 *
 * @param repository Not <code>NULL</code>.
 * @param id Not <code>NULL</code>.
 * @return Target group model or <code>NULL</code> if not found.
 */
ROX_INTERNAL TargetGroupModel *target_group_repository_get_target_group(
        TargetGroupRepository *repository,
        const char *id);

/**
 * @param repository Not <code>NULL</code>.
 */
ROX_INTERNAL void target_group_repository_free(TargetGroupRepository *repository);
