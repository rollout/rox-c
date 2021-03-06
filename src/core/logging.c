#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "util.h"
#include "rox/logging.h"
#include "logging.h"

static bool ROX_LOGGING_PRINT_TIME = false;

static void _default_logging_handler(void *target, RoxLogMessage *message) {
    FILE *stream = message->level == RoxLogLevelDebug ? stdout : stderr;
#ifndef NDEBUG
    if (ROX_LOGGING_PRINT_TIME) {
        fprintf(stream,
                "%lu [%s:%d] (%s) %s\n",
                (long) current_time_millis(),
                message->file,
                message->line,
                message->level_name,
                message->message);
    } else {
        fprintf(stream,
                "[%s:%d] (%s) %s\n",
                message->file,
                message->line,
                message->level_name,
                message->message);
    }
#else
    if (ROX_LOGGING_PRINT_TIME) {
        fprintf(stream,
            "%lu (%s) %s\n",
            (long) current_time_millis(),
            message->level_name,
            message->message);
    } else {
        fprintf(stream,
            "(%s) %s\n",
            message->level_name,
            message->message);
    }

#endif
    fflush(stream);
}

static void *ROX_LOGGING_TARGET = NULL;
static rox_logging_handler ROX_LOGGING_HANDLER = &_default_logging_handler;
static RoxLogLevel ROX_MIN_LOGGING_LEVEL = RoxLogLevelError;

ROX_API void rox_logging_init(RoxLoggingConfig *config) {
    assert(config);
    ROX_LOGGING_TARGET = config->target;
    ROX_LOGGING_HANDLER = config->handler ? config->handler : &_default_logging_handler;
    ROX_MIN_LOGGING_LEVEL = config->min_level > 0 ? config->min_level : RoxLogLevelError;
    ROX_LOGGING_PRINT_TIME = config->print_time;
}

#define ROX_LOG_MESSAGE_BUFFER_SIZE 10240

static void _rox_handle_log_message(
        const char *file_name, int line, RoxLogLevel log_level, const char *fmt, va_list args) {

    RoxLogMessage message;
    message.level = log_level;
    message.file = file_name;
    message.line = line;

    switch (log_level) {
        case RoxLogLevelTrace:
            message.level_name = "TRACE";
            break;
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

    char *buffer = malloc(ROX_LOG_MESSAGE_BUFFER_SIZE);
    vsnprintf(buffer, ROX_LOG_MESSAGE_BUFFER_SIZE, fmt, args);
    message.message = buffer;
    ROX_LOGGING_HANDLER(ROX_LOGGING_TARGET, &message);
    free(buffer);
}

#undef ROX_LOG_MESSAGE_BUFFER_SIZE

ROX_INTERNAL void rox_log_trace(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelTrace) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelTrace, fmt, args);
            va_end(args);
}

ROX_INTERNAL void rox_log_debug(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelDebug) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelDebug, fmt, args);
            va_end(args);
}

ROX_INTERNAL void rox_log_warning(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelWarning) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelWarning, fmt, args);
            va_end(args);
}

ROX_INTERNAL void rox_log_error(const char *file_name, int line, const char *fmt, ...) {
    assert(fmt);
    if (ROX_MIN_LOGGING_LEVEL > RoxLogLevelError) {
        return;
    }
    va_list args;
            va_start(args, fmt);
    _rox_handle_log_message(file_name, line, RoxLogLevelError, fmt, args);
            va_end(args);
}
