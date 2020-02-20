#include <stdlib.h>
#include <assert.h>
#include <core/configuration/models.h>
#include "models.h"

//
// RoxExperiment
//

RoxExperiment *ROX_INTERNAL experiment_create(ExperimentModel *model) {
    assert(model);
    RoxExperiment *experiment = calloc(1, sizeof(RoxExperiment));
    experiment->identifier = model->id;
    experiment->name = model->name;
    experiment->archived = model->archived;
    experiment->labels = model->labels;
    experiment->stickiness_property = model->stickiness_property;
    return experiment;
}

void ROX_INTERNAL experiment_free(RoxExperiment *experiment) {
    assert(experiment);
    free(experiment);
}

//
// RoxReportingValue
//

RoxReportingValue *ROX_INTERNAL reporting_value_create(const char *name, const char *value) {
    RoxReportingValue *reporting_value = calloc(1, sizeof(RoxReportingValue));
    reporting_value->name = name;
    reporting_value->value = value;
    return reporting_value;
}

void ROX_INTERNAL reporting_value_free(RoxReportingValue *reporting_value) {
    assert(reporting_value);
    free(reporting_value);
}
