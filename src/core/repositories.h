#pragma once

#include <collectc/hashtable.h>
#include "configuration/models.h"
#include "properties.h"

//
// CustomPropertyRepository
//

typedef struct ROX_INTERNAL CustomPropertyRepository CustomPropertyRepository;

typedef void (*custom_property_handler)(CustomProperty *property);

/**
 * The returned object myst be freed after use by calling <code>custom_property_repository_free()</code>.
 */
CustomPropertyRepository *custom_property_repository_create();

void ROX_INTERNAL custom_property_repository_add_custom_property(
        CustomPropertyRepository *repository,
        CustomProperty *property);

void ROX_INTERNAL custom_property_repository_add_custom_property_if_not_exists(
        CustomPropertyRepository *repository,
        CustomProperty *property);

CustomProperty *ROX_INTERNAL custom_property_repository_get_custom_property(
        CustomPropertyRepository *repository,
        const char *property_name);

void ROX_INTERNAL custom_property_repository_get_all_custom_properties(
        CustomPropertyRepository *repository,
        HashTable **hash_table_ref);

void ROX_INTERNAL custom_property_repository_set_handler(
        CustomPropertyRepository *repository,
        custom_property_handler handler);

/**
 * @param repository Not NULL.
 */
void custom_property_repository_free(CustomPropertyRepository *repository);

//
// ExperimentRepository
//

typedef struct ROX_INTERNAL ExperimentRepository ExperimentRepository;

/**
 * The returned object must be destroyed after use by calling <code>experiment_repository_free()</code>.
 */
ExperimentRepository *experiment_repository_create();

/**
 * @param experiments List of <code>ExperimentModel *</code>.
 */
void ROX_INTERNAL experiment_repository_set_experiments(List *experiments);

/**
 * @param repository Not NULL.
 */
void ROX_INTERNAL experiment_repository_free(ExperimentRepository *repository);

/**
 * @param repository Not NULL.
 * @param flag_name Not NULL.
 * @return Experiment model or NULL if not found.
 */
ExperimentModel *ROX_INTERNAL experiment_repository_get_experiment_by_flag(
        ExperimentRepository *repository,
        const char *flag_name);

/**
 * @param repository Not NULL.
 * @return List of <code>ExperimentModel *</code>
 */
List *ROX_INTERNAL experiment_repository_get_all_experiments(ExperimentRepository *repository);

//
// FlagRepository
//

typedef struct ROX_INTERNAL FlagRepository FlagRepository;

FlagRepository *flag_repository_create();

// TODO

//
// TargetGroupRepository
//

// TODO
