#pragma once

#include <stdarg.h>
#include "rox/server.h"

typedef struct ErrorReporter ErrorReporter;

typedef void (*error_reporting_func)(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list ars);

typedef struct ErrorReporterConfig {
    void *target;
    error_reporting_func reporting_func;
} ErrorReporterConfig;

/**
 * @return Can be <code>NULL</code>.
 */
ROX_INTERNAL ErrorReporter *error_reporter_create(ErrorReporterConfig *config);

/**
 * @param reporter Not <code>NULL</code>.
 * @param fmt Not <code>NULL</code>.
 */
ROX_INTERNAL void error_reporter_report(ErrorReporter *reporter, const char *file, int line, const char *fmt, ...);

/**
 * @param reporter Not <code>NULL</code>.
 */
ROX_INTERNAL void error_reporter_free(ErrorReporter *reporter);
