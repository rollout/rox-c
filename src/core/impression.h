#pragma once

#include "roxapi.h"
#include "context.h"
#include "configuration/models.h"
#include "impression/models.h"

typedef struct ROX_INTERNAL ImpressionInvoker ImpressionInvoker;

typedef void ROX_INTERNAL (*impression_handler)(ReportingValue *value, Experiment *experiment, Context *context);

ImpressionInvoker* impression_invoker_create();

void ROX_INTERNAL impression_invoker_register(
        ImpressionInvoker *impression_invoker,
        impression_handler handler);

/**
 * @param impression_invoker Not <code>NULL</code>.
 * @param value Not <code>NULL</code>.
 * @param experiment May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 */
void ROX_INTERNAL impression_invoker_invoke(
        ImpressionInvoker *impression_invoker,
        ReportingValue *value,
        ExperimentModel *experiment,
        Context *context);

/**
 * @param impression_invoker Not <code>NULL</code>.
 */
void ROX_INTERNAL impression_invoker_free(ImpressionInvoker *impression_invoker);