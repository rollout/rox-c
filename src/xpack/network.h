#pragma once

#include "rollout.h"

#include "core/client.h"
#include "core/network.h"
#include "core/repositories.h"

//
// Debouncer
//

typedef struct ROX_INTERNAL Debouncer Debouncer;

typedef void ROX_INTERNAL (*debouncer_func)(void *target);

/**
 * @param interval_millis > 0
 * @param target Can be <code>NULL</code>.
 * @param func Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
Debouncer *ROX_INTERNAL debouncer_create(int interval_millis, void *target, debouncer_func func);

/**
 * @param debouncer Not <code>NULL</code>.
 */
void ROX_INTERNAL debouncer_invoke(Debouncer *debouncer);

/**
 * @param debouncer Not <code>NULL</code>.
 */
void ROX_INTERNAL debouncer_free(Debouncer *debouncer);

//
// StateSender
//

typedef struct ROX_INTERNAL StateSender StateSender;

/**
 * @param request Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param flag_repository Not <code>NULL</code>.
 * @param custom_property_repository Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
StateSender *ROX_INTERNAL state_sender_create(
        Request *request,
        DeviceProperties *device_properties,
        FlagRepository *flag_repository,
        CustomPropertyRepository *custom_property_repository);

/**
 * @param sender Not <code>NULL</code>.
 */
void ROX_INTERNAL state_sender_send(StateSender *sender);

/**
 * @param sender Not <code>NULL</code>.
 */
void ROX_INTERNAL state_sender_send_debounce(StateSender *sender);

/**
 * @param sender Not <code>NULL</code>.
 */
void ROX_INTERNAL state_sender_free(StateSender *sender);
