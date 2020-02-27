#pragma once

#include <stdbool.h>
#include "rollout.h"

//
// ExperimentModel
//

typedef struct ExperimentModel {
    char *id;
    char *name;
    char *condition;
    bool archived;
    RoxList *flags;
    RoxSet *labels;
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
 * @param flags List of strings. Can be NULL. If passed, ownership of this object is delegated to the experiment model.
 * @param labels Set of strings. Can be NULL. If passed, ownership of this object is delegated to the experiment model.
 * @param stickiness_property Can be <code>NULL</code>. If provided, the value will be copied internally. The caller holds an ownership on the passed pointer.
 */
ROX_INTERNAL ExperimentModel *experiment_model_create(
        const char *id,
        const char *name,
        const char *condition,
        bool archived,
        RoxList *flags,
        RoxSet *labels,
        const char *stickiness_property);

/**
 * @param model Not <code>NULL</code>.
 * @return Not <code>NULL</code>. Deep copy of the given <code>model</code>.
 */
ROX_INTERNAL ExperimentModel *experiment_model_copy(ExperimentModel *model);

/**
 * @param model Not <code>NULL</code>.
 */
ROX_INTERNAL void experiment_model_free(ExperimentModel *model);

//
// TargetGroupModel
//

typedef struct TargetGroupModel {
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
ROX_INTERNAL TargetGroupModel *target_group_model_create(
        const char *id,
        const char *condition);

/**
 * @param model Not <code>NULL</code>.
 * @retun Not <code>NULL</code>. Deep copy of the given <code>model</code>.
 */
ROX_INTERNAL TargetGroupModel *target_group_model_copy(TargetGroupModel *model);

/**
 * @param model Not <code>NULL</code>.
 */
ROX_INTERNAL void target_group_model_free(TargetGroupModel *model);
