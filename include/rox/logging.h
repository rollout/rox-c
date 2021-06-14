#pragma once

#include <rox/defs.h>

typedef enum RoxLogLevel {
    RoxLogLevelTrace = 1,
    RoxLogLevelDebug,
    RoxLogLevelWarning,
    RoxLogLevelError,
    RoxLogLevelNone
} RoxLogLevel;

typedef struct RoxLogMessage {
    const char *file;
    int line;
    RoxLogLevel level;
    const char *level_name;
    const char *message;
} RoxLogMessage;

typedef void (*rox_logging_handler)(void *target, RoxLogMessage *message);

typedef struct RoxLoggingConfig {
    RoxLogLevel min_level;
    void *target;
    rox_logging_handler handler;
    bool print_time;
} RoxLoggingConfig;

#define ROX_LOGGING_CONFIG_INITIALIZER(log_level) {log_level, NULL, NULL, false}

ROX_API void rox_logging_init(RoxLoggingConfig *config);
