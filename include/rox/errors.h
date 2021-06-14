#pragma once

typedef enum RoxStateCode {
    RoxUninitialized = 0,
    RoxSettingUp = 1,
    RoxInitialized = 2,
    RoxShuttingDown = 3,
    RoxErrorEmptyApiKey = -1,
    RoxErrorInvalidApiKey = -2,
    RoxErrorGenericSetupFailure = -1000
} RoxStateCode;
