#include <assert.h>
#include "reporting.h"
#include "properties.h"

struct ROX_INTERNAL ErrorReporter {
    error_reporting_func report;
};

void ROX_INTERNAL _error_reporter_report(ErrorReporter *reporter, const char *fmt, ...) {
    assert(reporter);
    assert(fmt);
    // TODO: port from X-Pack
}

ErrorReporter *ROX_INTERNAL error_reporter_create(ErrorReporterConfig *config) {
    ErrorReporter *reporter = calloc(1, sizeof(ErrorReporter));
    reporter->report = config->reporting_func ? config->reporting_func : &_error_reporter_report;
    return reporter;
}

void ROX_INTERNAL error_reporter_report(ErrorReporter *reporter, const char *fmt, ...) {
    assert(reporter);
    assert(fmt);
    reporter->report(reporter, fmt);
}

void ROX_INTERNAL error_reporter_free(ErrorReporter *reporter) {
    assert(reporter);
    free(reporter);
}
