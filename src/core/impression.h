#pragma once

#include "rox/impression.h"
#include "rox/context.h"
#include "configuration/models.h"
#include "impression/models.h"

typedef struct ImpressionInvoker ImpressionInvoker;

/**
 * Impression delegate is able to work with experiment,
 * while the public impression handle doesn't have experiment
 * argument passed.
 *
 * @param target Target object passed to <code>impression_invoker_set_delegate</code>. May be <code>NULL</code>.
 * @param value Impression value.
 * @param experiment Experiment used. May be <code>NULL</code>.
 * @param context Context used. May be <code>NULL</code>.
 */
typedef void (*impression_invoker_delegate)(
        void *target,
        RoxReportingValue *value,
        RoxExperiment *experiment,
        RoxContext *context);

ROX_INTERNAL ImpressionInvoker *impression_invoker_create();

/**
 * Register internal delegate for impression invoker. All impressions will
 * be first passed to the given delegate and then will be processed
 * by the registered handlers.
 *
 * @param impression_invoker Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param delegate Not <code>NULL</code>.
 */
ROX_INTERNAL void impression_invoker_set_delegate(
        ImpressionInvoker *impression_invoker,
        void *target,
        impression_invoker_delegate delegate);

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