#pragma once

#include "rox/server.h"
#include "context.h"
#include "configuration/models.h"
#include "impression/models.h"

typedef struct ImpressionInvoker ImpressionInvoker;

ImpressionInvoker *impression_invoker_create();

/**
 * @param impression_invoker Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_INTERNAL void impression_invoker_register(
        ImpressionInvoker *impression_invoker,
        void *target,
        rox_impression_handler handler);

/**
 * @param impression_invoker Not <code>NULL</code>.
 * @param value May be <code>NULL</code>.
 * @param experiment May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
ROX_INTERNAL void impression_invoker_invoke(
        ImpressionInvoker *impression_invoker,
        RoxReportingValue *value,
        ExperimentModel *experiment,
        RoxContext *context);

/**
 * @param impression_invoker Not <code>NULL</code>.
 */
ROX_INTERNAL void impression_invoker_free(ImpressionInvoker *impression_invoker);