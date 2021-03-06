#include <assert.h>
#include <stdlib.h>
#include "models.h"
#include "collections.h"

//
// ExperimentModel
//

ROX_INTERNAL ExperimentModel *experiment_model_create(
        const char *id,
        const char *name,
        const char *condition,
        bool archived,
        RoxList *flags,
        RoxSet *labels,
        const char *stickiness_property) {

    assert(id);
    assert(name);
    assert(condition);

    ExperimentModel *model = calloc(1, sizeof(ExperimentModel));
    model->id = mem_copy_str(id);
    model->name = mem_copy_str(name);
    model->condition = mem_copy_str(condition);
    model->archived = archived;
    model->flags = flags;
    model->labels = labels;
    if (stickiness_property) {
        model->stickiness_property = mem_copy_str(stickiness_property);
    }
    return model;
}

ROX_INTERNAL ExperimentModel *experiment_model_copy(ExperimentModel *model) {
    assert(model);
    return experiment_model_create(model->id,
                                   model->name,
                                   model->condition,
                                   model->archived,
                                   mem_deep_copy_list(model->flags, (void *(*)(void *)) &mem_copy_str),
                                   mem_deep_copy_set(model->labels, (void *(*)(void *)) &mem_copy_str),
                                   model->stickiness_property);
}

ROX_INTERNAL void experiment_model_free(ExperimentModel *model) {
    assert(model);
    free(model->id);
    free(model->name);
    free(model->condition);
    if (model->stickiness_property) {
        free(model->stickiness_property);
    }
    if (model->flags) {
        rox_list_free_cb(model->flags, &free);
    }
    if (model->labels) {
        RoxSetIter *iter = rox_set_iter_create();
        rox_set_iter_init(iter, model->labels);
        void *val;
        while (rox_set_iter_next(iter, &val)) {
            free(val);
        }
        rox_set_iter_free(iter);
        rox_set_free(model->labels);
    }
    free(model);
}

//
// TargetGroupModel
//

ROX_INTERNAL TargetGroupModel *target_group_model_create(
        const char *id,
        const char *condition) {
    assert(id);
    assert(condition);
    TargetGroupModel *model = calloc(1, sizeof(TargetGroupModel));
    model->id = mem_copy_str(id);
    model->condition = mem_copy_str(condition);
    return model;
}

ROX_INTERNAL TargetGroupModel *target_group_model_copy(TargetGroupModel *model) {
    assert(model);
    return target_group_model_create(model->id, model->condition);
}

ROX_INTERNAL void target_group_model_free(TargetGroupModel *model) {
    assert(model);
    free(model->id);
    free(model->condition);
    free(model);
}
