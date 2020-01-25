#pragma once

#include "roxapi.h"
#include "core/configuration/models.h"

//
// Experiment
//

typedef struct ROX_INTERNAL Experiment {
    char *name;
    char *identifier;
    bool archived;
    HashSet *labels;
} Experiment;

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
Experiment *ROX_INTERNAL experiment_create(ExperimentModel *model);

/**
 * @param experiment Not <code>NULL</code>.
 */
void ROX_INTERNAL experiment_free(Experiment *experiment);

//
// ReportingValue
//

typedef struct ROX_INTERNAL ReportingValue {
    char *name;
    char *value;
} ReportingValue;

/**
 * @param name May be <code>NULL</code>.
 * @param value May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ReportingValue *ROX_INTERNAL reporting_value_create(const char *name, const char *value);

void ROX_INTERNAL reporting_value_free(ReportingValue *reporting_value);

