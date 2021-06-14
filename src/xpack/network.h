#pragma once

#include "rox/server.h"

#include "core/client.h"
#include "core/network.h"
#include "core/repositories.h"

//
// Debouncer
//

typedef struct Debouncer Debouncer;

typedef void (*debouncer_func)(void *target);

/**
 * @param interval_millis > 0
 * @param target Can be <code>NULL</code>.
 * @param func Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL Debouncer *debouncer_create(int interval_millis, void *target, debouncer_func func);

/**
 * @param debouncer Not <code>NULL</code>.
 */
ROX_INTERNAL void debouncer_invoke(Debouncer *debouncer);

/**
 * @param debouncer Not <code>NULL</code>.
 */
ROX_INTERNAL void debouncer_free(Debouncer *debouncer);

//
// StateSender
//

typedef struct StateSender StateSender;

/**
 * @param request Not <code>NULL</code>.
 * @param device_properties Not <code>NULL</code>.
 * @param flag_repository Not <code>NULL</code>.
 * @param custom_property_repository Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL StateSender *state_sender_create(
        Request *request,
        DeviceProperties *device_properties,
        FlagRepository *flag_repository,
        CustomPropertyRepository *custom_property_repository);

/**
 * @param sender Not <code>NULL</code>.
 */
ROX_INTERNAL void state_sender_send(StateSender *sender);

/**
 * @param sender Not <code>NULL</code>.
 */
ROX_INTERNAL void state_sender_send_debounce(StateSender *sender);

/**
 * @param sender Not <code>NULL</code>.
 */
ROX_INTERNAL void state_sender_free(StateSender *sender);
