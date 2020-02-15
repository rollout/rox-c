#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "logging.h"

static void _default_logging_handler(void *target, RoxLogMessage *message) {
    fprintf(message->level == RoxLogLevelDebug
            ? stdout
            : stderr,
            "[%s] (%s:%d) %s\n",
            message->level_name,
            message->file,
            message->line,
            message->message);
}

static void *ROX_LOGGING_TARGET = NULL;
static rox_logging_handler ROX_LOGGING_HANDLER = &_default_logging_handler;
static RoxLogLevel ROX_MIN_LOGGING_LEVEL = RoxLogLevelError;

void ROX_API rox_logging_init(RoxLoggingConfig *config) {
    assert(config);
    ROX_LOGGING_TARGET = config->target;
    ROX_LOGGING_HANDLER = config->handler ? config->handler : &_default_logging_handler;
    ROX_MIN_LOGGING_LEVEL = config->min_level > 0 ? config->min_level : RoxLogLevelError;
}

#define ROX_LOG_MESSAGE_BUFFER_SIZE 1024

static void _rox_handle_log_message(
        const char *file_name, int line, RoxLogLevel log_level, const char *fmt, va_list args) {

    RoxLogMessage message;
    message.level = log_level;
    message.file = file_name;
    message.line = line;

    switch (log_level) {
        case RoxLogLevelDebug:
            message.level_name = "DEBUG";
            break;
        case RoxLogLevelWarning:
            message.level_name = "WARNING";
            break;
        case RoxLogLevelError:
            message.level_name = "ERROR";
            break;
        default:
            assert(false);
            return;
    }

    char buffer[ROX_LOG_MESSAGE_BUFFER_SIZE];
    vsprintf_s(buffer, ROX_LOG_MESSAGE_BUFFER_SIZE, fmt, args);
    message.message = buffer;
    ROX_LOGGING_HANDLER(ROX_LOGGING_TARGET, &message);
}

#undef ROX_LOG_MESSAGE_BUFFER_SIZE

void ROX_INTERNAL rox_log_debug(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelDebug) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelDebug, fmt, args);
            va_end(args);
}

void ROX_INTERNAL rox_log_warning(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelWarning) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelWarning, fmt, args);
            va_end(args);
}

void ROX_INTERNAL rox_log_error(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelError) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelError, fmt, args);
            va_end(args);
}
