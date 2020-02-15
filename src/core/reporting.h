#pragma once

#include <stdarg.h>
#include "roxapi.h"

typedef struct ROX_INTERNAL ErrorReporter ErrorReporter;

typedef void ROX_INTERNAL (*error_reporting_func)(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list ars);

typedef struct ROX_INTERNAL ErrorReporterConfig {
    void *target;
    error_reporting_func reporting_func;
} ErrorReporterConfig;

/**
 * @return Can be <code>NULL</code>.
 */
ErrorReporter *ROX_INTERNAL error_reporter_create(ErrorReporterConfig *config);

/**
 * @param reporter Not <code>NULL</code>.
 * @param fmt Not <code>NULL</code>.
 */
void ROX_INTERNAL error_reporter_report(ErrorReporter *reporter, const char *file, int line, const char *fmt, ...);

/**
 * @param reporter Not <code>NULL</code>.
 */
void ROX_INTERNAL error_reporter_free(ErrorReporter *reporter);
