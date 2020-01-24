#include <assert.h>
#include "core/configuration/models.h"
#include "roxx/parser.h"
#include "core/context.h"
#include "entities.h"
#include "util.h"

//
// Variant
//

struct ROX_INTERNAL Variant {
    char *default_value;
    List *options;
    char *condition;
    Parser *parser;
    Context *global_context;
    ImpressionInvoker *impression_invoker;
    ExperimentModel *experiment;
    char *name;
};

Variant *ROX_INTERNAL variant_create(const char *default_value, List *options) {
    assert(default_value);
    Variant *variant = calloc(1, sizeof(Variant));
    variant->default_value = mem_copy_str(default_value);
    variant->options = options;
    if (!variant->options) {
        list_new(&variant->options);
    }
    if (!str_in_list(variant->default_value, options)) {
        list_add(options, variant->default_value);
    }
    return variant;
}

void ROX_INTERNAL variant_set_for_evaluation(
        Variant *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker) {
    assert(variant);
    if (experiment) {
        variant->experiment = experiment;
        variant->condition = mem_copy_str(experiment->condition);
    } else {
        variant->experiment = NULL;
        variant->condition = mem_copy_str("");
    }
    variant->parser = parser;
    variant->impression_invoker = impression_invoker;
}

void ROX_INTERNAL variant_set_context(Variant *variant, Context *context) {
    assert(variant);
    assert(context);
    variant->global_context = context;
}

char *ROX_INTERNAL variant_get_name(Variant *variant) {
    assert(variant);
    return variant->name;
}

void ROX_INTERNAL variant_set_name(Variant *variant, const char *name) {
    assert(variant);
    assert(name);
    variant->name = mem_copy_str(name);
}

char *ROX_INTERNAL _variant_get_value(Variant *variant, Context *context, char *default_value) {
    assert(variant);
    char *return_value = default_value;
    Context *merged_context = context_create_merged(variant->global_context, context);
    if (variant->parser && !str_is_empty(variant->condition)) {
        EvaluationResult *evaluation_result = parser_evaluate_expression(
                variant->parser,
                variant->condition,
                merged_context);
        if (evaluation_result) {
            char *value = result_get_string(evaluation_result);
            if (!str_is_empty(value)) {
                return_value = value;
            }
        }
    }
    if (variant->impression_invoker) {
        ReportingValue *reporting_value = reporting_value_create(variant->name, return_value);
        impression_invoker_invoke(
                variant->impression_invoker,
                reporting_value,
                variant->experiment,
                merged_context);
        reporting_value_free(reporting_value);
    }
    context_free(merged_context);
    return return_value;
}

char *ROX_INTERNAL variant_get_value_or_default(Variant *variant, Context *context) {
    assert(variant);
    return _variant_get_value(variant, context, variant->default_value);
}

char *ROX_INTERNAL variant_get_value_or_null(Variant *variant, Context *context) {
    assert(variant);
    return _variant_get_value(variant, context, NULL);
}

void ROX_INTERNAL variant_free(Variant *variant) {
    assert(variant);
    if (variant->name) {
        free(variant->name);
    }
    if (variant->default_value) {
        free(variant->default_value);
    }
    if (variant->condition) {
        free(variant->condition);
    }
    if (variant->parser) {
        parser_free(variant->parser);
    }
    if (variant->global_context) {
        context_free(variant->global_context);
    }
    // TODO: free parser, global_context, experiment, and impression_invoker?
    free(variant);
}

//
// Flag
//

const char *ROX_INTERNAL FLAG_TRUE_VALUE = "true";
const char *ROX_INTERNAL FLAG_FALSE_VALUE = "false";

Variant *ROX_INTERNAL variant_create_flag() {
    return variant_create_flag_with_default(false);
}

Variant *ROX_INTERNAL variant_create_flag_with_default(bool default_value) {
    return variant_create(default_value ? FLAG_TRUE_VALUE : FLAG_FALSE_VALUE,
                          ROX_LIST(ROX_COPY(FLAG_TRUE_VALUE), ROX_COPY(FLAG_FALSE_VALUE)));
}
