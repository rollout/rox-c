#include <stdarg.h>
#include <assert.h>
#include "reporting.h"
#include "properties.h"

struct ROX_INTERNAL ErrorReporter {
    void *target;
    error_reporting_func report;
};

void ROX_INTERNAL _error_reporter_report(void *target, ErrorReporter *reporter, const char *fmt, va_list ars) {
    assert(reporter);
    assert(fmt);
    // TODO: port from X-Pack
}

ErrorReporter *ROX_INTERNAL error_reporter_create(ErrorReporterConfig *config) {
    ErrorReporter *reporter = calloc(1, sizeof(ErrorReporter));
    reporter->target = config && config->target ? config->target : NULL;
    reporter->report = config && config->reporting_func ? config->reporting_func : &_error_reporter_report;
    return reporter;
}

void ROX_INTERNAL error_reporter_report(ErrorReporter *reporter, const char *fmt, ...) {
    assert(reporter);
    assert(fmt);
    va_list args;
            va_start(args, fmt);
    reporter->report(reporter->target, reporter, fmt, args);
            va_end(args);
}

void ROX_INTERNAL error_reporter_free(ErrorReporter *reporter) {
    assert(reporter);
    free(reporter);
}
