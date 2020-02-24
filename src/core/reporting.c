#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include "reporting.h"
#include "core/logging.h"

struct ROX_INTERNAL ErrorReporter {
    void *target;
    error_reporting_func report;
};

#define ROX_ERROR_REPORT_MESSAGE_BUFFER_LENGTH 1024

void ROX_INTERNAL _error_reporter_report_dummy(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list args) {

    assert(reporter);
    assert(file);
    assert(line);
    assert(fmt);

    char buffer[ROX_ERROR_REPORT_MESSAGE_BUFFER_LENGTH];
    vsnprintf(buffer, ROX_ERROR_REPORT_MESSAGE_BUFFER_LENGTH, fmt, args);
    ROX_DEBUG("Dummy error report at %s:%d: %s", file, line, buffer);
}

#undef ROX_ERROR_REPORT_MESSAGE_BUFFER_LENGTH

ErrorReporter *ROX_INTERNAL error_reporter_create(ErrorReporterConfig *config) {
    ErrorReporter *reporter = calloc(1, sizeof(ErrorReporter));
    reporter->target = config && config->target ? config->target : NULL;
    reporter->report = config && config->reporting_func ? config->reporting_func : &_error_reporter_report_dummy;
    return reporter;
}

void ROX_INTERNAL error_reporter_report(
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt, ...) {

    assert(reporter);
    assert(file);
    assert(line);
    assert(fmt);

    va_list args;
            va_start(args, fmt);
    reporter->report(reporter->target, reporter, file, line, fmt, args);
            va_end(args);
}

void ROX_INTERNAL error_reporter_free(ErrorReporter *reporter) {
    assert(reporter);
    free(reporter);
}
