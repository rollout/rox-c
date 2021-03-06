#pragma once

#include "rox/flags.h"

// EvaluationContext

typedef struct EvaluationResult EvaluationResult;

typedef struct EvaluationContext EvaluationContext;

typedef RoxDynamicValue *(*from_string_converter_func)(const char *value);

typedef RoxDynamicValue *(*from_eval_result_converter_func)(EvaluationResult *result);

typedef char *(*to_string_converter_func)(RoxDynamicValue *value);

typedef struct FlagValueConverter {
    from_string_converter_func from_string;
    from_eval_result_converter_func from_eval_result;
    to_string_converter_func to_string;
} FlagValueConverter;

typedef RoxDynamicValue *(*variant_eval_func)(
        RoxStringBase *variant,
        const char *default_value,
        EvaluationContext *eval_context,
        FlagValueConverter *converter);

/**
 * @param variant May be <code>NULL</code>.
 * @param context May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL EvaluationContext *eval_context_create(RoxStringBase *variant, RoxContext *context);

ROX_INTERNAL RoxContext *eval_context_get_context(EvaluationContext *eval_context);

ROX_INTERNAL void eval_context_free(EvaluationContext *context);
