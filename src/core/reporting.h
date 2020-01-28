#pragma once

#include "roxapi.h"

typedef struct ROX_INTERNAL ErrorReporter ErrorReporter;

typedef void ROX_INTERNAL (*error_reporting_func)(ErrorReporter *reporter, const char *fmt, ...);

typedef struct ROX_INTERNAL ErrorReporterConfig {
    error_reporting_func reporting_func;
} ErrorReporterConfig;

/**
 * @return Not <code>NULL</code>.
 */
ErrorReporter *ROX_INTERNAL error_reporter_create(ErrorReporterConfig *config);

/**
 * @param reporter Not <code>NULL</code>.
 * @param fmt Not <code>NULL</code>.
 */
void ROX_INTERNAL error_reporter_report(ErrorReporter *reporter, const char *fmt, ...);

/**
 * @param reporter Not <code>NULL</code>.
 */
void ROX_INTERNAL error_reporter_free(ErrorReporter *reporter);
