#include <assert.h>
#include "core/configuration/models.h"
#include "roxx/parser.h"
#include "core/context.h"
#include "entities.h"
#include "repositories.h"
#include "util.h"

//
// RoxVariant
//

struct ROX_API RoxVariant {
    char *default_value;
    List *options;
    char *condition;
    Parser *parser;
    RoxContext *global_context;
    ImpressionInvoker *impression_invoker;
    ExperimentModel *experiment;
    char *name;
    bool is_flag;
};

RoxVariant *ROX_INTERNAL variant_create(const char *default_value, List *options) {
    RoxVariant *variant = calloc(1, sizeof(RoxVariant));
    variant->default_value = default_value ? mem_copy_str(default_value) : NULL;
    variant->options = options;
    if (!variant->options) {
        list_new(&variant->options);
    }
    if (variant->default_value && !str_in_list(variant->default_value, options)) {
        list_add(options, mem_copy_str(variant->default_value));
    }
    return variant;
}

bool ROX_INTERNAL variant_is_flag(RoxVariant *variant) {
    assert(variant);
    return variant->is_flag;
}

const char *ROX_INTERNAL variant_get_name(RoxVariant *variant) {
    assert(variant);
    return variant->name;
}

const char *ROX_INTERNAL variant_get_default_value(RoxVariant *variant) {
    assert(variant);
    return variant->default_value;
}

const char *ROX_INTERNAL variant_get_condition(RoxVariant *variant) {
    assert(variant);
    return variant->condition;
}

ExperimentModel *ROX_INTERNAL variant_get_experiment(RoxVariant *variant) {
    assert(variant);
    return variant->experiment;
}

Parser *ROX_INTERNAL variant_get_parser(RoxVariant *variant) {
    assert(variant);
    return variant->parser;
}

ImpressionInvoker *ROX_INTERNAL variant_get_impression_invoker(RoxVariant *variant) {
    assert(variant);
    return variant->impression_invoker;
}

List *ROX_INTERNAL variant_get_options(RoxVariant *variant) {
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
        variant->experiment = NULL;
    }
    if (variant->impression_invoker) {
        variant->impression_invoker = NULL;
    }
}

void ROX_INTERNAL variant_set_for_evaluation(
        RoxVariant *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker) {
    assert(variant);
    _variant_reset_evaluation_context(variant);
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

void ROX_INTERNAL variant_set_context(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    assert(context);
    variant->global_context = context;
}

void ROX_INTERNAL variant_set_name(RoxVariant *variant, const char *name) {
    assert(variant);
    assert(name);
    variant->name = mem_copy_str(name);
}

void ROX_INTERNAL variant_set_condition(RoxVariant *variant, const char *condition) {
    assert(variant);
    assert(condition);
    variant->condition = mem_copy_str(condition);
}

static char *_variant_get_value(RoxVariant *variant, RoxContext *context, char *default_value) {
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

char *ROX_INTERNAL variant_get_value_or_default(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    return _variant_get_value(variant, context, variant->default_value);
}

char *ROX_INTERNAL variant_get_value_or_null(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    return _variant_get_value(variant, context, NULL);
}

void ROX_INTERNAL variant_free(RoxVariant *variant) {
    assert(variant);
    _variant_reset_evaluation_context(variant);
    if (variant->global_context) {
        rox_context_free(variant->global_context);
    }
    if (variant->name) {
        free(variant->name);
    }
    if (variant->default_value) {
        free(variant->default_value);
    }
    if (variant->options) {
        list_destroy_cb(variant->options, &free);
    }
    free(variant);
}

//
// Flag
//

const char *ROX_INTERNAL FLAG_TRUE_VALUE = "true";
const char *ROX_INTERNAL FLAG_FALSE_VALUE = "false";

const bool FLAG_TRUE_VALUE_BOOL = true;
const bool FLAG_FALSE_VALUE_BOOL = false;

RoxVariant *ROX_INTERNAL variant_create_flag() {
    return variant_create_flag_with_default(false);
}

RoxVariant *ROX_INTERNAL variant_create_flag_with_default(bool default_value) {
    RoxVariant *flag = variant_create(default_value ? FLAG_TRUE_VALUE : FLAG_FALSE_VALUE,
                                      ROX_LIST(ROX_COPY(FLAG_FALSE_VALUE), ROX_COPY(FLAG_TRUE_VALUE)));
    flag->is_flag = true;
    return flag;
}

bool ROX_INTERNAL flag_is_enabled(RoxVariant *variant, RoxContext *context) {
    assert(variant);
    char *value = variant_get_value_or_default(variant, context);
    bool result = str_equals(value, FLAG_TRUE_VALUE);
    free(value);
    return result;
}

const bool *ROX_INTERNAL flag_is_enabled_or_null(RoxVariant *variant, RoxContext *context) {
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

void ROX_INTERNAL flag_enabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (flag_is_enabled(variant, context)) {
        action();
    }
}

void ROX_INTERNAL flag_disabled_do(RoxVariant *variant, RoxContext *context, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (!flag_is_enabled(variant, context)) {
        action();
    }
}

//
// FlagSetter
//

struct ROX_INTERNAL FlagSetter {
    FlagRepository *flag_repository;
    Parser *parser;
    ExperimentRepository *experiment_repository;
    ImpressionInvoker *impression_invoker;
};

void ROX_INTERNAL _flag_setter_repository_callback(void *target, RoxVariant *variant) {
    assert(target);
    assert(variant);
    FlagSetter *flag_setter = (FlagSetter *) target;
    ExperimentModel *exp = experiment_repository_get_experiment_by_flag(
            flag_setter->experiment_repository, variant->name);
    variant_set_for_evaluation(variant, flag_setter->parser, exp, flag_setter->impression_invoker);
}

FlagSetter *ROX_INTERNAL flag_setter_create(
        FlagRepository *flag_repository,
        Parser *parser,
        ExperimentRepository *experiment_repository,
        ImpressionInvoker *impression_invoker) {
    assert(flag_repository);
    assert(experiment_repository);
    FlagSetter *flag_setter = calloc(1, sizeof(struct FlagSetter));
    flag_setter->flag_repository = flag_repository;
    flag_setter->parser = parser;
    flag_setter->experiment_repository = experiment_repository;
    flag_setter->impression_invoker = impression_invoker;
    flag_repository_add_flag_added_callback(flag_repository, flag_setter, &_flag_setter_repository_callback);
    return flag_setter;
}

void ROX_INTERNAL flag_setter_set_experiments(FlagSetter *flag_setter) {
    assert(flag_setter);

    HashSet *flags_with_condition;
    hashset_new(&flags_with_condition);
    List *experiments = experiment_repository_get_all_experiments(flag_setter->experiment_repository);

    LIST_FOREACH(exp, experiments, {
        ExperimentModel *model = (ExperimentModel *) exp;
        ListIter flag_iter;
        list_iter_init(&flag_iter, model->flags);
        char *flag_name;
        while (list_iter_next(&flag_iter, (void **) &flag_name) != CC_ITER_END) {
            RoxVariant *flag = flag_repository_get_flag(flag_setter->flag_repository, flag_name);
            if (flag) {
                variant_set_for_evaluation(flag, flag_setter->parser, exp, flag_setter->impression_invoker);
                hashset_add(flags_with_condition, flag_name);
            }
        }
    })

    HashTable *all_flags = flag_repository_get_all_flags(flag_setter->flag_repository);
    TableEntry *entry;
    HASHTABLE_FOREACH(entry, all_flags, {
        RoxVariant *flag = entry->value;
        if (!hashset_contains(flags_with_condition, flag->name)) {
            variant_set_for_evaluation(flag, flag_setter->parser, NULL, flag_setter->impression_invoker);
        }
    })
}

void ROX_INTERNAL flag_setter_free(FlagSetter *flag_setter) {
    assert(flag_setter);
    free(flag_setter);
}

//
// EntitiesProvider
//

struct ROX_INTERNAL EntitiesProvider {
    int stub; // so that it has at least one member
};

EntitiesProvider *ROX_INTERNAL entities_provider_create() {
    return calloc(1, sizeof(EntitiesProvider));
}

RoxVariant *ROX_INTERNAL entities_provider_create_flag(EntitiesProvider *provider, bool default_value) {
    assert(provider);
    return variant_create_flag_with_default(default_value);
}

RoxVariant *ROX_INTERNAL entities_provider_create_variant(
        EntitiesProvider *provider,
        const char *defaultValue,
        List *options) {
    assert(provider);
    return variant_create(defaultValue, options);
}

void ROX_INTERNAL entities_provider_free(EntitiesProvider *provider) {
    assert(provider);
    free(provider);
}
