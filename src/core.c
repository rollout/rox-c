#include <assert.h>
#include <pthread.h>
#include <pcre2.h>
#include <errno.h>
#include <xpack/network.h>
#include "core.h"
#include "core/logging.h"
#include "eval/extensions.h"
#include "xpack/reporting.h"
#include "xpack/configuration.h"
#include "xpack/impression.h"
#include "os.h"

//
// PeriodicTask
//

typedef void (*periodic_task_func)(void *target);

typedef struct PeriodicTask {
    pthread_t thread;
    pthread_mutex_t thread_mutex;
    pthread_cond_t thread_cond;
    void *target;
    periodic_task_func func;
    int period_seconds;
    bool thread_started;
    bool stopped;
} PeriodicTask;

static void *periodic_task_thread_func(void *ptr) {
    PeriodicTask *task = (PeriodicTask *) ptr;
    if (!task->stopped) {
        int result;
        do {
            // The following code is an analogue of sleep() with the exception
            // that it allows the thread to be awakened by the state sender when
            // it's destroyed for example.
            struct timespec ts = get_future_timespec(task->period_seconds * 1000);
            pthread_mutex_lock(&task->thread_mutex);
            result = pthread_cond_timedwait(&task->thread_cond, &task->thread_mutex, &ts);
            pthread_mutex_unlock(&task->thread_mutex);
            if (result == ETIMEDOUT && !task->stopped) {
                task->func(task->target);
            }
        } while (result == ETIMEDOUT && !task->stopped);
    }
    task->thread_started = false;
#ifndef ROX_APPLE
    pthread_detach(pthread_self()); // free thread resources
#endif
    return NULL;
}

static PeriodicTask *periodic_task_create(int seconds, void *target, periodic_task_func func) {
    assert(seconds > 0);
    assert(func);
    PeriodicTask *task = calloc(1, sizeof(PeriodicTask));
    task->period_seconds = seconds;
    task->target = target;
    task->func = func;
    task->thread_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    task->thread_cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    task->thread_started = (pthread_create(
            &task->thread, NULL, periodic_task_thread_func, (void *) task) == 0);
    return task;
}

static void periodic_task_free(PeriodicTask *task) {
    assert(task);
    task->stopped = true;
    if (task->thread_started) {
        pthread_cancel(task->thread);
        pthread_join(task->thread, NULL);
    }
    free(task);
}

//
// Core
//

struct RoxCore {
    bool initialized;
    FlagRepository *flag_repository;
    CustomPropertyRepository *custom_property_repository;
    ExperimentRepository *experiment_repository;
    TargetGroupRepository *target_group_repository;
    FlagSetter *flag_setter;
    Parser *parser;
    ImpressionInvoker *impression_invoker;
    XImpressionInvoker *x_impression_invoker;
    ConfigurationFetchedInvoker *configuration_fetched_invoker;
    XConfigurationFetchedInvoker *x_configuration_fetched_invoker;
    DynamicProperties *dynamic_properties;
    SdkSettings *sdk_settings;
    DeviceProperties *device_properties;
    InternalFlags *internal_flags;
    ErrorReporter *error_reporter;
    XErrorReporter *x_error_reporter;
    ConfigurationFetcher *configuration_fetcher;
    SignatureVerifier *signature_verifier;
    APIKeyVerifier *api_key_verifier;
    StateSender *state_sender;
    AnalyticsClient *analytics_client;
    ConfigurationParser *configuration_parser;
    PeriodicTask *periodic_task;
    Request *configuration_fetcher_request;
    Request *state_sender_request;
    Request *report_request;
    BUID *buid;
    double last_fetch_time;
    ConfigurationFetchResult *last_configuration;
    pthread_mutex_t fetch_lock;
    bool stopped;
    RoxContext *global_context;
};

static void core_repository_callback(void *target, RoxStringBase *variant) {
    assert(target != NULL);
    assert(variant != NULL);
    RoxCore *core = (RoxCore *) target;
    if (core->global_context) {
        variant_set_context(variant, core->global_context);
    }
}

ROX_INTERNAL RoxCore *rox_core_create(RequestConfig *request_config) {

    RoxCore *core = calloc(1, sizeof(RoxCore));

    core->flag_repository = flag_repository_create();
    flag_repository_add_flag_added_callback(core->flag_repository, core, core_repository_callback);

    core->custom_property_repository = custom_property_repository_create();
    core->experiment_repository = experiment_repository_create();
    core->target_group_repository = target_group_repository_create();
    core->parser = parser_create();
    core->impression_invoker = impression_invoker_create();
    core->configuration_fetched_invoker = configuration_fetched_invoker_create();
    core->dynamic_properties = dynamic_properties_create();

    core->flag_setter = flag_setter_create(
            core->flag_repository,
            core->parser,
            core->experiment_repository,
            core->impression_invoker);

    core->configuration_fetcher_request = request_create(request_config);
    core->state_sender_request = request_create(request_config);
    core->report_request = request_create(request_config);

    parser_add_properties_extensions(core->parser, core->custom_property_repository, core->dynamic_properties);
    parser_add_experiments_extensions(core->parser, core->target_group_repository, core->flag_repository,
                                      core->experiment_repository);

    return core;
}

static bool core_check_throttle_interval(RoxCore *core, bool is_source_pushing) {
    int *fetch_throttle_interval = internal_flags_get_int_value(
            core->internal_flags, "rox.internal.throttleFetchInSeconds");
    if (fetch_throttle_interval) {
        if (*fetch_throttle_interval > 0 &&
            (!is_source_pushing ||
             internal_flags_is_enabled(core->internal_flags, "rox.internal.considerThrottleInPush"))) {
            double current_time = current_time_millis();
            if (current_time < core->last_fetch_time + *fetch_throttle_interval * 1000) {
                free(fetch_throttle_interval);
                return false;
            }
            core->last_fetch_time = current_time;
        }
        free(fetch_throttle_interval);
    }
    return true;
}

ROX_INTERNAL void rox_core_fetch(RoxCore *core, bool is_source_pushing) {
    if (!core->configuration_fetcher) {
        return;
    }

    pthread_mutex_lock(&core->fetch_lock);

    if (core->stopped) {
        ROX_DEBUG("ROX is stopped. Cancelling fetch");
        pthread_mutex_unlock(&core->fetch_lock);
        return;
    }

    if (!core_check_throttle_interval(core, is_source_pushing)) {
        ROX_WARN("Skipping fetch - kill switch");
        pthread_mutex_unlock(&core->fetch_lock);
        return;
    }

    ConfigurationFetchResult *result = configuration_fetcher_fetch(core->configuration_fetcher);
    if (!result) {
        pthread_mutex_unlock(&core->fetch_lock);
        return;
    }

    bool has_changes = true;
    if (core->last_configuration) {
        has_changes = !cJSON_Compare(result->parsed_data, core->last_configuration->parsed_data, true);
        configuration_fetch_result_free(core->last_configuration);
    }
    core->last_configuration = result;

    Configuration *configuration = configuration_parser_parse(core->configuration_parser, result);
    if (configuration) {

        experiment_repository_set_experiments(
                core->experiment_repository,
                mem_deep_copy_list(configuration->experiments,
                                   (void *(*)(void *)) &experiment_model_copy));

        target_group_repository_set_target_groups(
                core->target_group_repository,
                mem_deep_copy_list(configuration->target_groups,
                                   (void *(*)(void *)) &target_group_model_copy));

        flag_setter_set_experiments(core->flag_setter);

        configuration_fetched_invoker_invoke(
                core->configuration_fetched_invoker,
                result->source == CONFIGURATION_SOURCE_LOCAL_STORAGE
                ? AppliedFromLocalStorage
                : AppliedFromNetwork, // TODO: embedded?
                configuration->signature_date,
                has_changes);

        configuration_free(configuration);
        ROX_DEBUG(has_changes
                  ? "Configuration updated"
                  : "No changes in configuration");
    }

    pthread_mutex_unlock(&core->fetch_lock);
}

static void core_x_configuration_fetch_func(void *target) {
    assert(target);
    RoxCore *core = (RoxCore *) target;
    ROX_DEBUG("Fetching configuration");
    rox_core_fetch(core, true);
}

ROX_INTERNAL RoxStateCode rox_core_setup(
        RoxCore *core,
        SdkSettings *sdk_settings,
        DeviceProperties *device_properties,
        RoxOptions *rox_options) {

    assert(core);
    assert(sdk_settings);
    assert(device_properties);

    if (core->initialized) {
        ROX_WARN("Calling rox_core_setup second time?");
        return RoxInitialized;
    }

    if (pthread_mutex_init(&core->fetch_lock, NULL) != 0) {
        ROX_ERROR("mutex init has failed");
        return RoxErrorGenericSetupFailure;
    }

    const char *roxy_url = NULL;
    if (rox_options) {
        roxy_url = rox_options_get_roxy_url(rox_options);
        if (!roxy_url) {
            char *api_key = sdk_settings_get_api_key(sdk_settings);
            char *mongoIdPattern = "^[a-f\\d]{24}$";
            char *uuidIdPattern = "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$";
            if (!api_key || !api_key[0]) {
                ROX_ERROR("Invalid rollout apikey - must be specified");
                return RoxErrorEmptyApiKey;
            } else if (!str_matches(api_key, mongoIdPattern, PCRE2_CASELESS) && !str_matches(api_key, mongoIdPattern, PCRE2_CASELESS)) {
                ROX_ERROR("Illegal rollout apikey");
                return RoxErrorInvalidApiKey;
            }
        }
    }

    core->initialized = true;
    core->sdk_settings = sdk_settings;
    core->device_properties = device_properties;

    core->internal_flags = internal_flags_create(core->experiment_repository, core->parser);
    core->buid = buid_create(device_properties);
    core->x_error_reporter = x_error_reporter_create(core->report_request, device_properties, core->buid);

    ErrorReporterConfig error_reporter_config = {core->x_error_reporter, &x_error_reporter_report};
    core->error_reporter = error_reporter_create(&error_reporter_config);

    if (roxy_url) {

        core->signature_verifier = signature_verifier_create_dummy();
        core->api_key_verifier = api_key_verifier_create_dummy();

        core->configuration_fetcher = configuration_fetcher_create_roxy(
                core->configuration_fetcher_request, device_properties,
                core->buid,
                core->configuration_fetched_invoker,
                core->error_reporter,
                roxy_url);

    } else {

        core->signature_verifier = signature_verifier_create(NULL);

        APIKeyVerifierConfig api_key_verifier_config = {sdk_settings, NULL};
        core->api_key_verifier = api_key_verifier_create(&api_key_verifier_config);

        core->state_sender = state_sender_create(
                core->state_sender_request, device_properties, core->flag_repository,
                core->custom_property_repository);

        core->x_configuration_fetched_invoker = x_configuration_fetched_invoker_create(
                core->internal_flags,
                sdk_settings,
                core, &core_x_configuration_fetch_func);

        configuration_fetched_invoker_register_handler(
                core->configuration_fetched_invoker,
                core->x_configuration_fetched_invoker,
                &x_configuration_fetched_handler);

        core->configuration_fetcher = configuration_fetcher_create(
                core->configuration_fetcher_request,
                sdk_settings,
                device_properties,
                core->buid,
                core->configuration_fetched_invoker,
                core->error_reporter);

        AnalyticsClientConfig analytics_client_config = ANALYTICS_CLIENT_INITIAL_CONFIG;
        core->analytics_client = analytics_client_create(
                device_properties_get_rollout_key(device_properties),
                &analytics_client_config, device_properties);

        core->x_impression_invoker = x_impression_invoker_create(
                core->internal_flags,
                core->custom_property_repository,
                core->analytics_client);

        impression_invoker_set_delegate(core->impression_invoker,
                                        core->x_impression_invoker,
                                        &x_impression_handler_delegate);
    }

    if (rox_options) {
        rox_configuration_fetched_handler handler = rox_options_get_configuration_fetched_handler(rox_options);
        if (handler) {
            configuration_fetched_invoker_register_handler(
                    core->configuration_fetched_invoker,
                    rox_options_get_configuration_fetched_handler_target(rox_options),
                    handler);
        }
    }

    core->configuration_parser = configuration_parser_create(
            core->signature_verifier,
            core->error_reporter,
            core->api_key_verifier,
            core->configuration_fetched_invoker);

    rox_core_fetch(core, false);

    if (rox_options) {

        rox_impression_handler handler = rox_options_get_impression_handler(rox_options);
        if (handler) {
            impression_invoker_register(core->impression_invoker,
                                        rox_options_get_impression_handler_target(rox_options),
                                        handler);
        }

        int fetch_interval = rox_options_get_fetch_interval(rox_options);
        if (fetch_interval > 0) {
            core->periodic_task = periodic_task_create(fetch_interval, core, &core_x_configuration_fetch_func);
        }

        rox_dynamic_properties_rule rule = rox_options_get_dynamic_properties_rule(rox_options);
        if (rule) {
            dynamic_properties_set_rule(
                    core->dynamic_properties,
                    rox_options_get_dynamic_properties_rule_target(rox_options),
                    rule);
        }
    }

    if (core->state_sender) {
        state_sender_send_debounce(core->state_sender);
    }

    return RoxInitialized;
}

ROX_INTERNAL void rox_core_set_context(RoxCore *core, RoxContext *context) {
    assert(core);
    core->global_context = context;
    RoxMap *flags = flag_repository_get_all_flags(core->flag_repository);
    ROX_MAP_FOREACH(key, value, flags, {
        RoxStringBase *flag = (RoxStringBase *) value;
        variant_set_context(flag, context);
    })
}

ROX_INTERNAL void rox_core_add_flag(RoxCore *core, RoxStringBase *flag, const char *name) {
    assert(core);
    assert(flag);
    assert(name);
    if (flag_repository_get_flag(core->flag_repository, name)) {
        ROX_ERROR("Flag %s already registered", name);
        return;
    }
    flag_repository_add_flag(core->flag_repository, flag, name);
}

ROX_INTERNAL void rox_core_add_custom_property(RoxCore *core, CustomProperty *property) {
    assert(core);
    assert(property);
    custom_property_repository_add_custom_property(core->custom_property_repository, property);
}

ROX_INTERNAL void rox_core_add_custom_property_if_not_exists(RoxCore *core, CustomProperty *property) {
    assert(core);
    assert(property);
    custom_property_repository_add_custom_property_if_not_exists(core->custom_property_repository, property);
}

ROX_INTERNAL RoxDynamicApi *rox_core_create_dynamic_api(RoxCore *core, EntitiesProvider *entities_provider) {
    assert(core);
    assert(entities_provider);
    return dynamic_api_create(core->flag_repository, entities_provider);
}

ROX_INTERNAL void rox_core_free(RoxCore *core) {
    assert(core);

    core->stopped = true;

    if (core->x_configuration_fetched_invoker) {
        x_configuration_fetched_invoker_free(core->x_configuration_fetched_invoker);
    }

    flag_setter_free(core->flag_setter);
    configuration_fetched_invoker_free(core->configuration_fetched_invoker);
    flag_repository_free(core->flag_repository);
    parser_free(core->parser);
    target_group_repository_free(core->target_group_repository);
    experiment_repository_free(core->experiment_repository);
    custom_property_repository_free(core->custom_property_repository);
    impression_invoker_free(core->impression_invoker);
    dynamic_properties_free(core->dynamic_properties);
    request_free(core->configuration_fetcher_request);
    request_free(core->state_sender_request);
    request_free(core->report_request);

    if (core->signature_verifier) {
        signature_verifier_free(core->signature_verifier);
    }

    if (core->internal_flags) {
        internal_flags_free(core->internal_flags);
    }

    if (core->buid) {
        buid_free(core->buid);
    }

    if (core->x_error_reporter) {
        x_error_reporter_free(core->x_error_reporter);
    }

    if (core->error_reporter) {
        error_reporter_free(core->error_reporter);
    }

    if (core->configuration_fetcher) {
        configuration_fetcher_free(core->configuration_fetcher);
    }

    if (core->configuration_parser) {
        configuration_parser_free(core->configuration_parser);
    }

    if (core->api_key_verifier) {
        api_key_verifier_free(core->api_key_verifier);
    }

    if (core->state_sender) {
        state_sender_free(core->state_sender);
    }

    if (core->analytics_client) {
        analytics_client_free(core->analytics_client);
    }

    if (core->x_impression_invoker) {
        x_impression_invoker_free(core->x_impression_invoker);
    }

    if (core->periodic_task) {
        periodic_task_free(core->periodic_task);
    }

    if (core->last_configuration) {
        configuration_fetch_result_free(core->last_configuration);
    }

    if (core->initialized) {
        pthread_mutex_destroy(&core->fetch_lock);
    }

    free(core);
}

ROX_INTERNAL FlagRepository *rox_core_get_flag_repository(RoxCore *core) {
    assert(core);
    return core->flag_repository;
}

ROX_INTERNAL ConfigurationFetchedInvoker *rox_core_get_configuration_fetched_invoker(RoxCore *core) {
    assert(core);
    return core->configuration_fetched_invoker;
}
