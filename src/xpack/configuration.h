#pragma once

#include "roxapi.h"
#include "core/configuration.h"
#include "core/client.h"

//
// XConfigurationFetchedInvoker
//

typedef struct ROX_INTERNAL XConfigurationFetchedInvoker XConfigurationFetchedInvoker;

typedef void ROX_INTERNAL (*x_configuration_fetch_func)(void *target, bool is_source_pushing);

/**
 * @param flags Not <code>NULL</code>.
 * @param sdk_settings Not <code>NULL</code>.
 * @param fetch_target May be <code>NULL</code>.
 * @param fetch_func Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
XConfigurationFetchedInvoker *ROX_INTERNAL x_configuration_fetched_invoker_create(
        InternalFlags *flags,
        SdkSettings *sdk_settings,
        void *fetch_target,
        x_configuration_fetch_func fetch_func);

/**
 * @param invoker Not <code>NULL</code>.
 */
void ROX_INTERNAL x_configuration_fetched_invoker_free(XConfigurationFetchedInvoker *invoker);

/**
 * @param target Not <code>NULL</code>. Supposed to be a pointer to <code>XConfigurationFetchedInvoker*</code>.
 * @param args Not <code>NULL</code>.
 */
void ROX_INTERNAL x_configuration_fetched_handler(void *target, ConfigurationFetchedArgs *args);