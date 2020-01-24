#pragma once

#include <stdbool.h>
#include <collectc/hashset.h>
#include <collectc/list.h>
#include "roxapi.h"

//
// ExperimentModel
//

typedef struct ROX_INTERNAL ExperimentModel {
    char *id;
    char *name;
    char *condition;
    bool archived;
    List *flags;
    HashSet *labels;
    char *stickiness_property;
} ExperimentModel;

/**
 * The caller is an owner of the returned object
 * and must destroy it using experiment_model_free();
 *
 * @param id Not <code>NULL</code>. Will be copied internally. The caller holds an ownership.
 * @param name Not <code>NULL</code>. Will be copied internally. The caller holds an ownership.
 * @param condition Not <code>NULL</code>. Will be copied internally. The caller holds an ownership.
 * @param archived
 * @param flags List of strings. Can be NULL. If passed, ownership of this object is delegated is NOT delegated to the experiment. Instead, the shallow copy of the list is created.
 * @param labels Set of strings. Can be NULL. If passed, ownership of this object is delegated is NOT delegated to the experiment. Instead, the shallow copy of the list is created.
 * @param stickiness_property Can be <code>NULL</code>. If provided, the value will be copied internally. The caller holds an ownership on the passed pointer.
 */
ExperimentModel *ROX_INTERNAL experiment_model_create(
        const char *id,
        const char *name,
        const char *condition,
        bool archived,
        List *flags,
        HashSet *labels,
        const char *stickiness_property);

/**
 * @param model Not <code>NULL</code>.
 */
void ROX_INTERNAL experiment_model_free(ExperimentModel *model);

//
// TargetGroupModel
//

typedef struct ROX_INTERNAL TargetGroupModel {
    char *id;
    char *condition;
} TargetGroupModel;

/**
 * The caller is an owner of the returned object
 * and must destroy it using target_group_model_free();
 *
 * @param id Not <code>NULL</code>. Will be copied internally. The caller holds an ownership.
 * @param condition Not <code>NULL</code>. Will be copied internally. The caller holds an ownership.
 */
TargetGroupModel *ROX_INTERNAL target_group_model_create(
        const char *id,
        const char *condition);

/**
 * @param model Not <code>NULL</code>.
 */
void ROX_INTERNAL target_group_model_free(TargetGroupModel *model);
