#pragma once

#include <rox/macros.h>
#include <rox/context.h>

typedef struct RoxReportingValue {
    const char *name;
    const char *value;
    bool targeting;
} RoxReportingValue;

/**
 * Note the reporting <code>value</code> may be freed right after call,
 * but its <code>name</code> and <code>value</code> can live longer.
 * It's recommended to make a copy if you need to use it sometimes later.
 *
 * @param target Can be <code>NULL</code>.
 * @param value Can be <code>NULL</code>.
 * @param experiment Can be <code>NULL</code>.
 * @param context Can be <code>NULL</code>.
 */
typedef void (*rox_impression_handler)(
        void *target,
        RoxReportingValue *value,
        RoxContext *context);
