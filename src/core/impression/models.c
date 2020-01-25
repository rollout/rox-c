#include <stdlib.h>
#include <assert.h>
#include <core/configuration/models.h>
#include "models.h"
#include "util.h"

//
// Experiment
//

Experiment *ROX_INTERNAL experiment_create(ExperimentModel *model) {
    assert(model);
    Experiment *experiment = calloc(1, sizeof(Experiment));
    experiment->identifier = model->id;
    experiment->name = model->name;
    experiment->archived = model->archived;
    experiment->labels = model->labels;
    return experiment;
}

void ROX_INTERNAL experiment_free(Experiment *experiment) {
    assert(experiment);
    free(experiment);
}

//
// ReportingValue
//

ReportingValue *ROX_INTERNAL reporting_value_create(const char *name, const char *value) {
    ReportingValue *reporting_value = calloc(1, sizeof(ReportingValue));
    reporting_value->name = name ? mem_copy_str(name) : NULL;
    reporting_value->value = value ? mem_copy_str(value) : NULL;
    return reporting_value;
}

void ROX_INTERNAL reporting_value_free(ReportingValue *reporting_value) {
    assert(reporting_value);
    if (reporting_value->name) {
        free(reporting_value->name);
    }
    if (reporting_value->value) {
        free(reporting_value->value);
    }
    free(reporting_value);
}
