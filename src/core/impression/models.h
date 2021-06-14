#pragma once

#include "rox/impression.h"
#include "core/configuration/models.h"

//
// RoxExperiment
//

typedef struct RoxExperiment {
    char *name;
    char *identifier;
    bool archived;
    RoxSet *labels;
    char *stickiness_property;
} RoxExperiment;

/**
 * The data is NOT copied internally, instead it copies the pointers.
 * This means that the caller is still responsible for freeing data after use
 * which should be done by calling <code>experiment_model_free()</code> and
 * <code>experiment_free()</code>. Note you cannot use the returned <code>experiment</code>
 * once the passed <code>model</code> is destroyed.
 *
 * @param model Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxExperiment *experiment_create(ExperimentModel *model);

/**
 * @param model Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxExperiment *experiment_copy(RoxExperiment *experiment);

/**
 * @param experiment Not <code>NULL</code>.
 */
ROX_INTERNAL void experiment_free(RoxExperiment *experiment);

//
// RoxReportingValue
//

/**
 * @param name May be <code>NULL</code>.
 * @param value May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxReportingValue *reporting_value_create(const char *name, const char *value, bool targeting);

/**
 * @param value Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL RoxReportingValue *reporting_value_copy(RoxReportingValue *value);

ROX_INTERNAL void reporting_value_free(RoxReportingValue *reporting_value);

