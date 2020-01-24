#pragma once

#include "roxapi.h"

typedef struct ReportingValue {
    char *name;
    char *value;
} ReportingValue;

ReportingValue *ROX_INTERNAL reporting_value_create(const char *name, const char *value);

void ROX_INTERNAL reporting_value_free(ReportingValue *reporting_value);
