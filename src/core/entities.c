#include <assert.h>
#include "core/configuration/models.h"
#include "roxx/parser.h"
#include "entities.h"
#include "repositories.h"
#include "util.h"
#include "collections.h"

//
// RoxVariant
//

struct RoxVariant {
    char *default_value;
    RoxList *options;
    char *condition;
    Parser *parser;
    RoxContext *global_context;
    ImpressionInvoker *impression_invoker;
    ExperimentModel *experiment;
    char *name;
    bool is_flag;
};

ROX_INTERNAL RoxVariant *variant_create(const char *default_value, RoxList *options) {
    RoxVariant *variant = calloc(1, sizeof(RoxVariant));
    variant->default_value = default_value ? mem_copy_str(default_value) : NULL;
    variant->options = options;
    if (!variant->options) {
        variant->options = rox_list_create();
    }
    if (variant->default_value && !str_in_list(variant->default_value, options)) {
        rox_list_add(options, mem_copy_str(variant->default_value));
    }
    return variant;
}

ROX_INTERNAL bool variant_is_flag(RoxVariant *variant) {
    assert(variant);
    return variant->is_flag;
}

ROX_INTERNAL const char *variant_get_name(RoxVariant *variant) {
    assert(variant);
    return variant->name;
}

ROX_INTERNAL const char *variant_get_default_value(RoxVariant *variant) {
    assert(variant);
    return variant->default_value;
}

ROX_INTERNAL const char *variant_get_condition(RoxVariant *variant) {
    assert(variant);
    return variant->condition;
}

ROX_INTERNAL ExperimentModel *variant_get_experiment(RoxVariant *variant) {
    assert(variant);
    return variant->experiment;
}

ROX_INTERNAL Parser *variant_get_parser(RoxVariant *variant) {
    assert(variant);
    return variant->parser;
}

ROX_INTERNAL ImpressionInvoker *variant_get_impression_invoker(RoxVariant *variant) {
    assert(variant);
    return variant->impression_invoker;
}

ROX_INTERNAL RoxList *variant_get_options(RoxVariant *variant) {
    assert(variant);
    return variant->options;
}

static void _variant_reset_evaluation_context(RoxVariant *variant) {
    assert(variant);
    if (variant->condition) {
        free(variant->condition);
        variant->condition = NULL;
    }
    if (variant->parser) {
        variant->parser = NULL;
    }
    if (variant->experiment) {
        experiment_model_free(variant->experiment);
        variant->experiment = NULL;
    }
    if (variant->impression_invoker) {
        variant->impression_invoker = NULL;
    }
}

ROX_INTERNAL void variant_set_for_evaluation(
        RoxVariant *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker) {
    assert(variant);
    _variant_reset_evaluation_context(variant);
    if (experiment) {
        variant->experiment = experiment_model_copy(experiment);
        variant->condition = mem_copy_str(experiment->condition);
    } else {
        variant->experiment = NULL;
        variant->condition = mem_copy_str("");
    }
    variant->parser = parser;
    variant->impression_invoker = impression_invoker;
}

ROX_INTERNAL void variant_set_context(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    variant->global_context = context;
}

ROX_INTERNAL void variant_set_name(RoxVariant *variant, const char *name) {
    assert(variant);
    assert(name);
    if (variant->name) {
        free(variant->name);
    }
    variant->name = mem_copy_str(name);
}

ROX_INTERNAL void variant_set_condition(RoxVariant *variant, const char *condition) {
    assert(variant);
    assert(condition);
    if (variant->condition) {
        free(variant->condition);
    }
    variant->condition = mem_copy_str(condition);
}

static char *_variant_get_value(RoxVariant *variant, RoxContext *context, const char *default_value) {
    assert(variant);
    bool value_set = false;
    char *return_value = NULL;
    RoxContext *merged_context = rox_context_create_merged(variant->global_context, context);
    if (variant->parser && !str_is_empty(variant->condition)) {
        EvaluationResult *evaluation_result = parser_evaluate_expression(
                variant->parser,
                variant->condition,
                merged_context);
        if (evaluation_result) {
            char *value = result_get_string(evaluation_result);
            if (!str_is_empty(value)) {
                return_value = mem_copy_str(value);
                value_set = true;
            }
            result_free(evaluation_result);
        }
    }
    if (!return_value && !value_set && default_value) {
        return_value = mem_copy_str(default_value);
    }
    if (variant->impression_invoker) {
        RoxReportingValue *reporting_value = reporting_value_create(variant->name, return_value);
        impression_invoker_invoke(
                variant->impression_invoker,
                reporting_value,
                variant->experiment,
                merged_context);
        reporting_value_free(reporting_value);
    }
    rox_context_free(merged_context);
    return return_value;
}

ROX_INTERNAL char *variant_get_value_or_default(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    return _variant_get_value(variant, context, variant->default_value);
}

ROX_INTERNAL char *variant_get_value_or(RoxVariant *variant, RoxContext *context, const char *default_value) {
    assert(variant);
    return _variant_get_value(variant, context, default_value);
}

ROX_INTERNAL char *variant_get_value_or_null(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    return _variant_get_value(variant, context, NULL);
}

ROX_INTERNAL void variant_free(RoxVariant *variant) {
    assert(variant);
    _variant_reset_evaluation_context(variant);
    if (variant->name) {
        free(variant->name);
    }
    if (variant->default_value) {
        free(variant->default_value);
    }
    if (variant->options) {
        rox_list_free_cb(variant->options, &free);
    }
    if (variant->experiment) {
        experiment_model_free(variant->experiment);
    }
    free(variant);
}

//
// Flag
//

ROX_INTERNAL const char *FLAG_TRUE_VALUE = "true";
ROX_INTERNAL const char *FLAG_FALSE_VALUE = "false";

const bool FLAG_TRUE_VALUE_BOOL = true;
const bool FLAG_FALSE_VALUE_BOOL = false;

ROX_INTERNAL RoxVariant *variant_create_flag() {
    return variant_create_flag_with_default(false);
}

ROX_INTERNAL RoxVariant *variant_create_flag_with_default(bool default_value) {
    RoxVariant *flag = variant_create(default_value ? FLAG_TRUE_VALUE : FLAG_FALSE_VALUE,
                                      ROX_LIST(ROX_COPY(FLAG_FALSE_VALUE), ROX_COPY(FLAG_TRUE_VALUE)));
    flag->is_flag = true;
    return flag;
}

ROX_INTERNAL bool flag_is_enabled(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    char *value = variant_get_value_or_default(variant, context);
    bool result = str_equals(value, FLAG_TRUE_VALUE);
    free(value);
    return result;
}

ROX_INTERNAL const bool *flag_is_enabled_or_null(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    char *value = variant_get_value_or_null(variant, context);
    if (!value) {
        return NULL;
    }
    const bool *result = str_equals(value, FLAG_TRUE_VALUE)
                         ? &FLAG_TRUE_VALUE_BOOL
                         : &FLAG_FALSE_VALUE_BOOL;
    free(value);
    return result;
}

ROX_INTERNAL bool flag_is_enabled_or(RoxVariant *variant, RoxContext *context, bool default_value) {
    assert(variant);
    char *value = variant_get_value_or(variant, context, default_value ? FLAG_TRUE_VALUE : FLAG_FALSE_VALUE);
    bool result = str_equals(value, FLAG_TRUE_VALUE);
    free(value);
    return result;
}

ROX_INTERNAL void flag_enabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (flag_is_enabled(variant, context)) {
        action();
    }
}

ROX_INTERNAL void flag_disabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (!flag_is_enabled(variant, context)) {
        action();
    }
}

//
// FlagSetter
//

struct FlagSetter {
    FlagRepository *flag_repository;
    Parser *parser;
    ExperimentRepository *experiment_repository;
    ImpressionInvoker *impression_invoker;
};

ROX_INTERNAL void _flag_setter_repository_callback(void *target, RoxVariant *variant) {
    assert(target);
    assert(variant);
    FlagSetter *flag_setter = (FlagSetter *) target;
    ExperimentModel *exp = experiment_repository_get_experiment_by_flag(
            flag_setter->experiment_repository, variant->name);
    variant_set_for_evaluation(variant, flag_setter->parser, exp, flag_setter->impression_invoker);
}

ROX_INTERNAL FlagSetter *flag_setter_create(
        FlagRepository *flag_repository,
        Parser *parser,
        ExperimentRepository *experiment_repository,
        ImpressionInvoker *impression_invoker) {
    assert(flag_repository);
    assert(experiment_repository);
    FlagSetter *flag_setter = calloc(1, sizeof(FlagSetter));
    flag_setter->flag_repository = flag_repository;
    flag_setter->parser = parser;
    flag_setter->experiment_repository = experiment_repository;
    flag_setter->impression_invoker = impression_invoker;
    flag_repository_add_flag_added_callback(flag_repository, flag_setter, &_flag_setter_repository_callback);
    return flag_setter;
}

ROX_INTERNAL void flag_setter_set_experiments(FlagSetter *flag_setter) {
    assert(flag_setter);

    RoxSet *flags_with_condition = rox_set_create();
    RoxList *experiments = experiment_repository_get_all_experiments(flag_setter->experiment_repository);

    ROX_LIST_FOREACH(exp, experiments, {
        ExperimentModel *model = (ExperimentModel *) exp;
        RoxListIter *flag_iter = rox_list_iter_create();
        rox_list_iter_init(flag_iter, model->flags);
        char *flag_name;
        while (rox_list_iter_next(flag_iter, (void **) &flag_name)) {
            RoxVariant *flag = flag_repository_get_flag(flag_setter->flag_repository, flag_name);
            if (flag) {
                variant_set_for_evaluation(flag, flag_setter->parser, exp, flag_setter->impression_invoker);
                rox_set_add(flags_with_condition, flag_name);
            }
        }
        rox_list_iter_free(flag_iter);
    })

    RoxMap *all_flags = flag_repository_get_all_flags(flag_setter->flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxVariant *flag = value;
        if (!rox_set_contains(flags_with_condition, flag->name)) {
            variant_set_for_evaluation(flag, flag_setter->parser, NULL, flag_setter->impression_invoker);
        }
    })

    rox_set_free(flags_with_condition);
}

ROX_INTERNAL void flag_setter_free(FlagSetter *flag_setter) {
    assert(flag_setter);
    free(flag_setter);
}

//
// EntitiesProvider
//

struct EntitiesProvider {
    int stub; // so that it has at least one member
};

ROX_INTERNAL EntitiesProvider *entities_provider_create() {
    return calloc(1, sizeof(EntitiesProvider));
}

ROX_INTERNAL RoxVariant *entities_provider_create_flag(EntitiesProvider *provider, bool default_value) {
    assert(provider);
    return variant_create_flag_with_default(default_value);
}

ROX_INTERNAL RoxVariant *entities_provider_create_variant(
        EntitiesProvider *provider,
        const char *defaultValue,
        RoxList *options) {
    assert(provider);
    return variant_create(defaultValue, options);
}

ROX_INTERNAL void entities_provider_free(EntitiesProvider *provider) {
    assert(provider);
    free(provider);
}
