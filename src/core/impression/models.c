#include <stdlib.h>
#include <assert.h>
#include "models.h"
#include "util.h"

ReportingValue *ROX_INTERNAL reporting_value_create(const char *name, const char *value) {
    assert(name);
    assert(value);
    ReportingValue *reporting_value = calloc(1, sizeof(ReportingValue));
    reporting_value->name = mem_copy_str(name);
    reporting_value->value = mem_copy_str(value);
    return reporting_value;
}

void ROX_INTERNAL reporting_value_free(ReportingValue *reporting_value) {
    assert(reporting_value);
    free(reporting_value->name);
    free(reporting_value->value);
    free(reporting_value);
}
