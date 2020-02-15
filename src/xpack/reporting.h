#pragma once

#include "roxapi.h"
#include "core/reporting.h"
#include "core/client.h"
#include "core/network.h"

typedef struct ROX_INTERNAL XErrorReporter XErrorReporter;

/**
 * @param request Not <code>NULL</code>.
 * @param properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
XErrorReporter *ROX_INTERNAL x_error_reporter_create(
        Request *request,
        DeviceProperties *properties,
        BUID *buid);

/**
 * @param reporter Not <code>NULL</code>.
 */
void ROX_INTERNAL x_error_reporter_free(XErrorReporter *reporter);

void ROX_INTERNAL x_error_reporter_report(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list args);
