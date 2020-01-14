#pragma once

#include <stdbool.h>
#include "visibility.h"

int *ROX_INTERNAL mem_copy_int(int value);

double *ROX_INTERNAL mem_copy_double(double value);

bool *ROX_INTERNAL mem_copy_bool(bool value);

char *ROX_INTERNAL mem_copy_str(const char *ptr);

int *ROX_INTERNAL str_to_int(char *str);

double *ROX_INTERNAL str_to_double(char *str);

