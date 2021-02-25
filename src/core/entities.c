#include <assert.h>
#include "core/configuration/models.h"
#include "roxx/parser.h"
#include "entities.h"
#include "repositories.h"
#include "util.h"
#include "collections.h"

//
// RoxStringBase
//

struct RoxStringBase {
    char *default_value;
    RoxList *options;
    char *condition;
    Parser *parser;
    RoxContext *global_context;
    ImpressionInvoker *impression_invoker;
    ExperimentModel *experiment;
    char *name;
    bool is_flag;
    bool is_string;
    bool is_int;
    bool is_double;
};

static RoxStringBase *variant_create_ptr(char *default_value, RoxList *options) {
    RoxStringBase *variant = calloc(1, sizeof(RoxStringBase));
    variant->default_value = default_value;
    variant->options = options;
    if (!variant->options) {
        variant->options = rox_list_create();
    }
    if (variant->default_value && !str_in_list(variant->default_value, variant->options)) {
        rox_list_add(variant->options, mem_copy_str(variant->default_value));
    }
    return variant;
}

ROX_INTERNAL RoxStringBase *variant_create_string(const char *default_value, RoxList *options) {
    RoxStringBase *variant = variant_create_ptr(default_value
                                                 ? mem_copy_str(default_value)
                                                 : NULL, options);
    variant->is_string = true;
    return variant;
}

ROX_INTERNAL RoxStringBase *variant_create_int(int defaultValue, RoxList *options) {
    RoxStringBase *variant = variant_create_ptr(mem_int_to_str(defaultValue), options);
    variant->is_int = true;
    return variant;
}

ROX_INTERNAL RoxStringBase *variant_create_double(double defaultValue, RoxList *options) {
    RoxStringBase *variant = variant_create_ptr(mem_double_to_str(defaultValue), options);
    variant->is_double = true;
    return variant;
}

ROX_INTERNAL bool variant_is_flag(RoxStringBase *variant) {
    assert(variant);
    return variant->is_flag;
}

ROX_INTERNAL bool variant_is_string(RoxStringBase *variant) {
    assert(variant);
    return variant->is_string;
}

ROX_INTERNAL bool variant_is_int(RoxStringBase *variant) {
    assert(variant);
    return variant->is_int;
}

ROX_INTERNAL bool variant_is_double(RoxStringBase *variant) {
    assert(variant);
    return variant->is_double;
}

ROX_INTERNAL const char *variant_get_name(RoxStringBase *variant) {
    assert(variant);
    return variant->name;
}

ROX_INTERNAL const char *variant_get_default_value(RoxStringBase *variant) {
    assert(variant);
    return variant->default_value;
}

ROX_INTERNAL const char *variant_get_condition(RoxStringBase *variant) {
    assert(variant);
    return variant->condition;
}

ROX_INTERNAL ExperimentModel *variant_get_experiment(RoxStringBase *variant) {
    assert(variant);
    return variant->experiment;
}

ROX_INTERNAL Parser *variant_get_parser(RoxStringBase *variant) {
    assert(variant);
    return variant->parser;
}

ROX_INTERNAL ImpressionInvoker *variant_get_impression_invoker(RoxStringBase *variant) {
    assert(variant);
    return variant->impression_invoker;
}

ROX_INTERNAL RoxList *variant_get_options(RoxStringBase *variant) {
    assert(variant);
    return variant->options;
}

static void variant_reset_evaluation_context(RoxStringBase *variant) {
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
        RoxStringBase *variant,
        Parser *parser,
        ExperimentModel *experiment,
        ImpressionInvoker *impression_invoker) {
    assert(variant);
    variant_reset_evaluation_context(variant);
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

ROX_INTERNAL void variant_set_context(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    variant->global_context = context;
}

ROX_INTERNAL void variant_set_name(RoxStringBase *variant, const char *name) {
    assert(variant);
    assert(name);
    if (variant->name) {
        free(variant->name);
    }
    variant->name = mem_copy_str(name);
}

ROX_INTERNAL void variant_set_condition(RoxStringBase *variant, const char *condition) {
    assert(variant);
    assert(condition);
    if (variant->condition) {
        free(variant->condition);
    }
    variant->condition = mem_copy_str(condition);
}

// Flag evaluation

typedef struct FlagEvaluationContext FlagEvaluationContext;

typedef RoxDynamicValue *(*from_string_converter_func)(const char *value);

typedef RoxDynamicValue *(*from_eval_result_converter_func)(EvaluationResult *result);

typedef char *(*to_string_converter_func)(RoxDynamicValue *value);

static RoxDynamicValue *string_to_string_value(const char *str) {
    if (str) {
        return rox_dynamic_value_create_string_copy(str);
    }
    return NULL;
}

static RoxDynamicValue *result_to_string_value(EvaluationResult *result) {
    char *value = result_get_string(result);
    if (value) {
        return rox_dynamic_value_create_string_copy(value);
    }
    return NULL;
}

static char *string_value_to_string(RoxDynamicValue *value) {
    return mem_copy_str(rox_dynamic_value_get_string(value));
}

static RoxDynamicValue *string_to_int_value(const char *str) {
    int *parsed = mem_str_to_int(str);
    if (parsed) {
        return rox_dynamic_value_create_int_ptr(parsed);
    }
    return NULL;
}

static char *int_value_to_string(RoxDynamicValue *value) {
    if (!rox_dynamic_value_is_int(value)) {
        return NULL;
    }
    int num = rox_dynamic_value_get_int(value);
    return mem_int_to_str(num);
}

static RoxDynamicValue *result_to_int_value(EvaluationResult *result) {
    int *int_ptr = result_get_int(result);
    if (int_ptr) {
        return rox_dynamic_value_create_int(*int_ptr);
    }
    char *str = result_get_string(result);
    if (str) {
        int *parsed = mem_str_to_int(str);
        if (parsed) {
            return rox_dynamic_value_create_int_ptr(parsed);
        }
    }
    return NULL;
}

static RoxDynamicValue *string_to_double_value(const char *str) {
    double *parsed = mem_str_to_double(str);
    if (parsed) {
        return rox_dynamic_value_create_double_ptr(parsed);
    }
    return NULL;
}

static char *double_value_to_string(RoxDynamicValue *value) {
    if (!rox_dynamic_value_is_double(value)) {
        return NULL;
    }
    double num = rox_dynamic_value_get_double(value);
    return mem_double_to_str(num);
}

static RoxDynamicValue *result_to_double_value(EvaluationResult *result) {
    double *dbl_ptr = result_get_double(result);
    if (dbl_ptr) {
        return rox_dynamic_value_create_double(*dbl_ptr);
    }
    int *int_ptr = result_get_int(result);
    if (int_ptr) {
        return rox_dynamic_value_create_double(*int_ptr);
    }
    char *str = result_get_string(result);
    if (str) {
        double *parsed = mem_str_to_double(str);
        if (parsed) {
            return rox_dynamic_value_create_double_ptr(parsed);
        }
    }
    return NULL;
}

static RoxDynamicValue *string_to_bool_value(const char *str) {
    return rox_dynamic_value_create_boolean(
            str != NULL
            ? str_equals(str, FLAG_TRUE_VALUE)
            : false);
}

static char *bool_value_to_string(RoxDynamicValue *value) {
    if (rox_dynamic_value_is_boolean(value)) {
        return rox_dynamic_value_get_boolean(value)
               ? mem_copy_str(FLAG_TRUE_VALUE)
               : mem_copy_str(FLAG_FALSE_VALUE);
    }
    return NULL;
}

static RoxDynamicValue *result_to_bool_value(EvaluationResult *result) {
    if (result_is_undefined(result)) {
        return NULL;
    }
    bool *bool_ptr = result_get_boolean(result);
    if (bool_ptr) {
        return rox_dynamic_value_create_boolean(*bool_ptr);
    }
    char *str = result_get_string(result);
    if (str) {
        bool bool_val = str_equals(str, FLAG_TRUE_VALUE);
        return rox_dynamic_value_create_boolean(bool_val);
    }
    return NULL;
}

struct FlagEvaluationContext {
    const char *default_value;
    from_string_converter_func from_string;
    from_eval_result_converter_func from_eval_result;
    to_string_converter_func to_string;
};

static FlagEvaluationContext *flag_eval_context_create(
        const char *default_value,
        from_string_converter_func from_string,
        from_eval_result_converter_func from_eval_result,
        to_string_converter_func to_string) {

    FlagEvaluationContext *context = calloc(1, sizeof(FlagEvaluationContext));
    context->default_value = default_value;
    context->from_string = from_string;
    context->from_eval_result = from_eval_result;
    context->to_string = to_string;
    return context;
}

static FlagEvaluationContext *flag_eval_context_create_string(const char *default_value) {
    return flag_eval_context_create(
            default_value,
            string_to_string_value,
            result_to_string_value,
            string_value_to_string);
}

static FlagEvaluationContext *flag_eval_context_create_int(const char *default_value) {
    return flag_eval_context_create(
            default_value,
            string_to_int_value,
            result_to_int_value,
            int_value_to_string);
}

static FlagEvaluationContext *flag_eval_context_create_double(const char *default_value) {
    return flag_eval_context_create(
            default_value,
            string_to_double_value,
            result_to_double_value,
            double_value_to_string);
}

static FlagEvaluationContext *flag_eval_context_create_bool(const char *default_value) {
    return flag_eval_context_create(
            default_value,
            string_to_bool_value,
            result_to_bool_value,
            bool_value_to_string);
}

static void flag_eval_context_free(FlagEvaluationContext *context) {
    free(context);
}

static RoxDynamicValue *variant_get_value(
        RoxStringBase *variant,
        RoxContext *context,
        FlagEvaluationContext *eval_context) {

    assert(variant);
    assert(eval_context);

    RoxContext *used_context = NULL;
    RoxDynamicValue *ret_val = NULL;

    RoxContext *merged_context = rox_context_create_merged(variant->global_context, context);
    if (variant->parser && !str_is_empty(variant->condition)) {
        EvaluationResult *evaluation_result = parser_evaluate_expression(
                variant->parser,
                variant->condition,
                merged_context);
        if (evaluation_result) {
            ret_val = eval_context->from_eval_result(evaluation_result);
            if (ret_val) {
                used_context = result_get_context(evaluation_result);
            }
            result_free(evaluation_result);
        }
    }
    if (!ret_val) {
        ret_val = eval_context->from_string(eval_context->default_value);
    }
    if (variant->impression_invoker) {
        char *string_value = eval_context->to_string(ret_val);
        RoxReportingValue *reporting_value = reporting_value_create(variant->name, string_value);
        impression_invoker_invoke(
                variant->impression_invoker,
                reporting_value,
                variant->experiment,
                used_context);
        reporting_value_free(reporting_value);
        free(string_value);
    }
    rox_context_free(merged_context);
    return ret_val;
}

// String flag

ROX_INTERNAL char *variant_get_string_or_default(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    return variant_get_string_or(variant, context, variant->default_value);
}

ROX_INTERNAL char *variant_get_string_or_null(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    return variant_get_string_or(variant, context, NULL);
}

ROX_INTERNAL char *variant_get_string_or(RoxStringBase *variant, RoxContext *context, const char *default_value) {
    assert(variant);
    FlagEvaluationContext *eval_context = flag_eval_context_create_string(default_value);
    RoxDynamicValue *value = variant_get_value(variant, context, eval_context);
    char *return_value = mem_copy_str(rox_dynamic_value_get_string(value));
    rox_dynamic_value_free(value);
    flag_eval_context_free(eval_context);
    return return_value;
}

ROX_INTERNAL void variant_free(RoxStringBase *variant) {
    assert(variant);
    variant_reset_evaluation_context(variant);
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

// Integer flag

static int variant_get_int(RoxStringBase *variant, RoxContext *context, const char* default_value) {
    assert(variant);
    FlagEvaluationContext *eval_context = flag_eval_context_create_int(default_value);
    RoxDynamicValue *value = variant_get_value(variant, context, eval_context);
    int return_value = rox_dynamic_value_get_int(value);
    rox_dynamic_value_free(value);
    flag_eval_context_free(eval_context);
    return return_value;
}

ROX_INTERNAL int variant_get_int_or_default(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    return variant_get_int(variant, context, variant->default_value);
}

ROX_INTERNAL int variant_get_int_or(RoxStringBase *variant, RoxContext *context, int default_value) {
    assert(variant);
    char *default_value_str = mem_int_to_str(default_value);
    int result = variant_get_int(variant, context, default_value_str);
    free(default_value_str);
    return result;
}

// Double flag

static double variant_get_double(RoxStringBase *variant, RoxContext *context, const char *default_value) {
    assert(variant);
    FlagEvaluationContext *eval_context = flag_eval_context_create_double(default_value);
    RoxDynamicValue *value = variant_get_value(variant, context, eval_context);
    double return_value = rox_dynamic_value_get_double(value);
    rox_dynamic_value_free(value);
    flag_eval_context_free(eval_context);
    return return_value;
}

ROX_INTERNAL double variant_get_double_or_default(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    return variant_get_double(variant, context, variant->default_value);
}

ROX_INTERNAL double variant_get_double_or(RoxStringBase *variant, RoxContext *context, double default_value) {
    assert(variant);
    char *default_value_str = mem_double_to_str(default_value);
    double result = variant_get_double(variant, context, default_value_str);
    free(default_value_str);
    return result;
}

//
// Boolean flag
//

ROX_INTERNAL const char *FLAG_TRUE_VALUE = "true";
ROX_INTERNAL const char *FLAG_FALSE_VALUE = "false";

const bool FLAG_TRUE_VALUE_BOOL = true;
const bool FLAG_FALSE_VALUE_BOOL = false;

ROX_INTERNAL RoxStringBase *variant_create_flag() {
    return variant_create_flag_with_default(false);
}

ROX_INTERNAL RoxStringBase *variant_create_flag_with_default(bool default_value) {
    RoxStringBase *flag = variant_create_ptr(mem_copy_str(default_value ? FLAG_TRUE_VALUE : FLAG_FALSE_VALUE),
                                         ROX_LIST(ROX_COPY(FLAG_FALSE_VALUE), ROX_COPY(FLAG_TRUE_VALUE)));
    flag->is_flag = true;
    return flag;
}

static bool variant_get_bool(RoxStringBase *variant, RoxContext *context, const char *default_value) {
    assert(variant);
    FlagEvaluationContext *eval_context = flag_eval_context_create_bool(default_value);
    RoxDynamicValue *value = variant_get_value(variant, context, eval_context);
    bool return_value = rox_dynamic_value_get_boolean(value);
    rox_dynamic_value_free(value);
    flag_eval_context_free(eval_context);
    return return_value;
}

ROX_INTERNAL bool flag_is_enabled(RoxStringBase *variant, RoxContext *context) {
    assert(variant);
    return variant_get_bool(variant, context, variant->default_value);
}

ROX_INTERNAL bool flag_is_enabled_or(RoxStringBase *variant, RoxContext *context, bool default_value) {
    assert(variant);
    char *default_value_str = mem_bool_to_str(default_value, FLAG_TRUE_VALUE, FLAG_FALSE_VALUE);
    bool result = variant_get_bool(variant, context, default_value_str);
    free(default_value_str);
    return result;
}

ROX_INTERNAL void flag_enabled_do(RoxStringBase *variant, RoxContext *context, void* target, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (flag_is_enabled(variant, context)) {
        action(target);
    }
}

ROX_INTERNAL void flag_disabled_do(RoxStringBase *variant, RoxContext *context, void* target, rox_flag_action action) {
    assert(variant);
    assert(action);
    if (!flag_is_enabled(variant, context)) {
        action(target);
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

static void flag_setter_repository_callback(void *target, RoxStringBase *variant) {
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
    flag_repository_add_flag_added_callback(flag_repository, flag_setter, &flag_setter_repository_callback);
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
            RoxStringBase *flag = flag_repository_get_flag(flag_setter->flag_repository, flag_name);
            if (flag) {
                variant_set_for_evaluation(flag, flag_setter->parser, exp, flag_setter->impression_invoker);
                rox_set_add(flags_with_condition, flag_name);
            }
        }
        rox_list_iter_free(flag_iter);
    })

    RoxMap *all_flags = flag_repository_get_all_flags(flag_setter->flag_repository);
    ROX_MAP_FOREACH(key, value, all_flags, {
        RoxStringBase *flag = value;
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

ROX_INTERNAL RoxStringBase *entities_provider_create_flag(EntitiesProvider *provider, bool default_value) {
    assert(provider);
    return variant_create_flag_with_default(default_value);
}

ROX_INTERNAL RoxStringBase *entities_provider_create_string(
        EntitiesProvider *provider,
        const char *defaultValue,
        RoxList *options) {
    assert(provider);
    return variant_create_string(defaultValue, options);
}

ROX_INTERNAL RoxStringBase *entities_provider_create_int(
        EntitiesProvider *provider,
        int defaultValue,
        RoxList *options) {
    assert(provider);
    return variant_create_int(defaultValue, options);
}

ROX_INTERNAL RoxStringBase *entities_provider_create_double(
        EntitiesProvider *provider,
        double defaultValue,
        RoxList *options) {
    assert(provider);
    return variant_create_double(defaultValue, options);
}

ROX_INTERNAL void entities_provider_free(EntitiesProvider *provider) {
    assert(provider);
    free(provider);
}
