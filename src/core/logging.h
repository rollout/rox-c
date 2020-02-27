#pragma once

#include <stdlib.h>
#include "rollout.h"

ROX_INTERNAL void rox_log_debug(const char *file_name, int line, const char *fmt, ...);

ROX_INTERNAL void rox_log_warning(const char *file_name, int line, const char *fmt, ...);

ROX_INTERNAL void rox_log_error(const char *file_name, int line, const char *fmt, ...);

#define ROX_DEBUG(fmt, ...) rox_log_debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ROX_WARN(fmt, ...) rox_log_warning(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define ROX_ERROR(fmt, ...) rox_log_error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
