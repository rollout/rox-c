#include <stdlib.h>
#include <assert.h>
#include "core/configuration/models.h"
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

RoxExperiment *ROX_INTERNAL experiment_copy(RoxExperiment *experiment) {
    assert(experiment);
    RoxExperiment *copy = calloc(1, sizeof(RoxExperiment));
    copy->identifier = experiment->identifier;
    copy->name = experiment->name;
    copy->archived = experiment->archived;
    copy->labels = experiment->labels;
    copy->stickiness_property = experiment->stickiness_property;
    return copy;
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

RoxReportingValue *ROX_INTERNAL reporting_value_copy(RoxReportingValue *value) {
    assert(value);
    return reporting_value_create(value->name, value->value);
}

void ROX_INTERNAL reporting_value_free(RoxReportingValue *reporting_value) {
    assert(reporting_value);
    free(reporting_value);
}
