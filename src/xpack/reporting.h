#pragma once

#include "rollout.h"
#include "core/reporting.h"
#include "core/client.h"
#include "core/network.h"

typedef struct XErrorReporter XErrorReporter;

/**
 * @param request Not <code>NULL</code>.
 * @param properties Not <code>NULL</code>.
 * @param buid Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL XErrorReporter *x_error_reporter_create(
        Request *request,
        DeviceProperties *properties,
        BUID *buid);

/**
 * @param reporter Not <code>NULL</code>.
 */
ROX_INTERNAL void x_error_reporter_free(XErrorReporter *reporter);

ROX_INTERNAL void x_error_reporter_report(
        void *target,
        ErrorReporter *reporter,
        const char *file,
        int line,
        const char *fmt,
        va_list args);
