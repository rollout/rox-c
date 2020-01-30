#include <assert.h>
#include <math.h>
#include "core/entities.h"
#include "extensions.h"
#include "util.h"

typedef struct ROX_INTERNAL ExperimentExtensionsContext {
    TargetGroupRepository *target_groups_repository;
    FlagRepository *flags_repository;
    ExperimentRepository *experiment_repository;
} ExperimentExtensionsContext;

void ROX_INTERNAL _parser_extensions_disposal_handler(void *target, Parser *parser) {
    assert(target);
    assert(parser);
    free(target);
}

double ROX_INTERNAL experiment_extensions_get_bucket(const char *seed) {
    assert(seed);
    unsigned char bytes[16];
    md5_str_b(seed, bytes);
    long hash = (bytes[0] & 0xFF) | ((bytes[1] & 0xFF) << 8) | ((bytes[2] & 0xFF) << 16) | ((bytes[3] & 0xFF) << 24);
    hash &= 0xffffffffL;
    double bucket = hash / (pow(2, 32) - 1);
    if (bucket == 1) {
        bucket = 0;
    }
    return bucket;
}

void ROX_INTERNAL _parser_operator_merge_seed(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    char *seed1 = rox_stack_get_string(item1);
    char *seed2 = rox_stack_get_string(item2);
    char *merged = mem_str_format("%s.%s", seed1, seed2);
    rox_stack_push_string_ptr(stack, merged);
}

void ROX_INTERNAL _parser_operator_is_in_percentage(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    double percentage = rox_stack_get_double(item1);
    char *seed = rox_stack_get_string(item2);
    double bucket = experiment_extensions_get_bucket(seed);
    bool is_in_percentage = bucket <= percentage;
    rox_stack_push_boolean(stack, is_in_percentage);
}

void ROX_INTERNAL
_parser_operator_is_in_percentage_range(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    StackItem *item1 = rox_stack_pop(stack);
    StackItem *item2 = rox_stack_pop(stack);
    StackItem *item3 = rox_stack_pop(stack);
    double percentage_low = rox_stack_get_double(item1);
    double percentage_high = rox_stack_get_double(item2);
    char *seed = rox_stack_get_string(item3);
    double bucket = experiment_extensions_get_bucket(seed);
    bool is_in_percentage = bucket >= percentage_low && bucket < percentage_high;
    rox_stack_push_boolean(stack, is_in_percentage);
}

void ROX_INTERNAL _parser_operator_flag_value(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    ExperimentExtensionsContext *extensions = (ExperimentExtensionsContext *) target;
    StackItem *item = rox_stack_pop(stack);
    char *feature_flag_identifier = rox_stack_get_string(item);
    bool value_set = false;
    char *result = NULL;
    Variant *variant = flag_repository_get_flag(extensions->flags_repository, feature_flag_identifier);
    if (variant) {
        result = variant_get_value_or_default(variant, context);
        value_set = true;
    } else {
        ExperimentModel *flags_experiment = experiment_repository_get_experiment_by_flag(
                extensions->experiment_repository, feature_flag_identifier);
        if (flags_experiment && !str_is_empty(flags_experiment->condition)) {
            EvaluationResult *evaluation_result = parser_evaluate_expression(
                    parser, flags_experiment->condition, context);
            char *result_str = result_get_string(evaluation_result);
            if (!str_is_empty(result_str)) {
                result = result_str;
                value_set = true;
            }
        }
    }
    if (!result && !value_set) {
        result = mem_copy_str(FLAG_FALSE_VALUE);
    }
    rox_stack_push_string_ptr(stack, result);
}

void ROX_INTERNAL
_parser_operator_is_in_target_group(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);
    ExperimentExtensionsContext *extensions = (ExperimentExtensionsContext *) target;
    StackItem *item = rox_stack_pop(stack);
    char *target_group_identifier = rox_stack_get_string(item);
    TargetGroupModel *target_group = target_group_repository_get_target_group(
            extensions->target_groups_repository, target_group_identifier);
    if (!target_group) {
        rox_stack_push_boolean(stack, false);
        return;
    }
    EvaluationResult *result = parser_evaluate_expression(parser, target_group->condition, context);
    bool *bool_result = result_get_boolean(result);
    rox_stack_push_boolean(stack, bool_result ? *bool_result : false);
}

void ROX_INTERNAL parser_add_experiments_extensions(
        Parser *parser,
        TargetGroupRepository *target_groups_repository,
        FlagRepository *flags_repository,
        ExperimentRepository *experiment_repository) {

    assert(parser);
    assert(target_groups_repository);
    assert(flags_repository);
    assert(experiment_repository);

    ExperimentExtensionsContext *context = calloc(1, sizeof(ExperimentExtensionsContext));
    context->target_groups_repository = target_groups_repository;
    context->flags_repository = flags_repository;
    context->experiment_repository = experiment_repository;

    // dispose this context when parser is destroyed
    parser_add_disposal_handler(parser, context, &_parser_extensions_disposal_handler);
    parser_add_operator(parser, "mergeSeed", context, &_parser_operator_merge_seed);
    parser_add_operator(parser, "isInPercentage", context, &_parser_operator_is_in_percentage);
    parser_add_operator(parser, "isInPercentageRange", context, &_parser_operator_is_in_percentage_range);
    parser_add_operator(parser, "flagValue", context, &_parser_operator_flag_value);
    parser_add_operator(parser, "isInTargetGroup", context, &_parser_operator_is_in_target_group);
}

typedef struct ROX_INTERNAL PropertiesExtensionsContext {
    CustomPropertyRepository *custom_property_repository;
    DynamicProperties *dynamic_properties;
} PropertiesExtensionsContext;

void ROX_INTERNAL _parser_operator_property(void *target, Parser *parser, CoreStack *stack, Context *context) {
    assert(parser);
    assert(stack);

    PropertiesExtensionsContext *extensions = (PropertiesExtensionsContext *) target;

    StackItem *item = rox_stack_pop(stack);
    const char *prop_name = rox_stack_get_string(item);
    CustomProperty *property = custom_property_repository_get_custom_property(extensions->custom_property_repository,
                                                                              prop_name);

    if (!property) {
        dynamic_properties_rule rule = dynamic_properties_get_rule(extensions->dynamic_properties);
        if (rule) {
            DynamicValue *value = rule(prop_name, context);
            if (value) {
                if (dynamic_value_is_string(value)) {
                    rox_stack_push_dynamic_value(stack, value);
                    return;
                } else if (dynamic_value_is_boolean(value)) {
                    rox_stack_push_dynamic_value(stack, value);
                    return;
                } else if (dynamic_value_is_int(value)) {
                    rox_stack_push_double(stack, dynamic_value_get_int(value));
                    return;
                } else if (dynamic_value_is_double(value)) {
                    rox_stack_push_dynamic_value(stack, value);
                    return;
                }
            }
        }
        rox_stack_push_undefined(stack);
        return;
    }

    DynamicValue *value = custom_property_get_value(property, context);
    if (value) {
        rox_stack_push_dynamic_value(stack, value);
        return;
    }

    rox_stack_push_undefined(stack);
}

void ROX_INTERNAL parser_add_properties_extensions(
        Parser *parser,
        CustomPropertyRepository *custom_property_repository,
        DynamicProperties *dynamic_properties) {

    assert(parser);
    assert(custom_property_repository);
    assert(dynamic_properties);

    PropertiesExtensionsContext *context = calloc(1, sizeof(PropertiesExtensionsContext));
    context->custom_property_repository = custom_property_repository;
    context->dynamic_properties = dynamic_properties;

    // dispose this context when parser is destroyed
    parser_add_disposal_handler(parser, context, &_parser_extensions_disposal_handler);
    parser_add_operator(parser, "property", context, &_parser_operator_property);
}
