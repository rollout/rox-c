#pragma once

#include <limits.h>
#include <float.h>

#include "rox/api.h"

typedef struct RoxMap RoxMap;

typedef struct RoxList RoxList;

typedef struct RoxSet RoxSet;

ROX_API RoxList *rox_list_create_va(void *skip, ...);

ROX_API RoxList *rox_list_create_str_va(void *skip, ...);

ROX_API RoxList *rox_list_create_int_va(void *skip, ...);

ROX_API RoxList *rox_list_create_double_va(void *skip, ...);

ROX_API RoxSet *rox_set_create_va(void *skip, ...);

ROX_API RoxMap *rox_map_create_va(void *skip, ...);

ROX_API char *mem_copy_str(const char *ptr);

#define ROX_LIST(...) rox_list_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_LIST ROX_LIST(NULL)

#define ROX_LIST_COPY_STR(...) rox_list_create_str_va(NULL, __VA_ARGS__, NULL)

#define ROX_INT_LIST(...) rox_list_create_int_va(NULL, __VA_ARGS__, INT_MIN)

#define ROX_DBL_LIST(...) rox_list_create_double_va(NULL, __VA_ARGS__, DBL_MIN)

#define ROX_SET(...) rox_set_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_SET ROX_SET(NULL)

#define ROX_COPY(str) mem_copy_str(str)

#define ROX_MAP(...) rox_map_create_va(NULL, __VA_ARGS__, NULL)

#define ROX_EMPTY_MAP ROX_MAP(NULL)