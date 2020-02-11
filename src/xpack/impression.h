#pragma once

#include "roxapi.h"
#include "core/reporting.h"
#include "core/entities.h"
#include "core/properties.h"
#include "core/client.h"
#include "xpack/analytics/client.h"

//
// XImpressionInvoker
//

typedef struct ROX_INTERNAL XImpressionInvoker XImpressionInvoker;

/**
 * @param flags Not <code>NULL</code>.
 * @param custom_property_repository Not <code>NULL</code>.
 * @param client Can be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
XImpressionInvoker *ROX_INTERNAL x_impression_invoker_create(
        InternalFlags *flags,
        CustomPropertyRepository *custom_property_repository,
        AnalyticsClient *client);

/**
 * @param invoker Not <code>NULL</code>.
 */
void ROX_INTERNAL x_impression_invoker_free(XImpressionInvoker *invoker);

/**
 * @param target Not <code>NULL</code>. Pointer to <code>XImpressionInvoker*</code>.
 */
void ROX_INTERNAL x_impression_handler(
        void *target,
        ReportingValue *value,
        Experiment *experiment,
        Context *context);
