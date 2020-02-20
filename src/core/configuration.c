#include <assert.h>
#include <collectc/list.h>
#include "configuration.h"
#include "configuration/models.h"
#include "core/logging.h"
#include "xpack/security.h"
#include "util.h"

//
// Configuration
//

Configuration *ROX_INTERNAL configuration_create(
        List *experiments,
        List *target_groups,
        const char *signature_date) {
    assert(experiments);
    assert(target_groups);
    assert(signature_date);
    Configuration *configuration = calloc(1, sizeof(Configuration));
    configuration->experiments = experiments;
    configuration->target_groups = target_groups;
    configuration->signature_date = mem_copy_str(signature_date);
    return configuration;
}

bool ROX_INTERNAL configuration_equals(Configuration *c1, Configuration *c2) {
    assert(c1);
    assert(c2);
    if (!str_equals(c1->signature_date, c2->signature_date)) {
        return false;
    }
    return str_list_equals(c1->experiments, c2->experiments) &&
           str_list_equals(c1->target_groups, c2->target_groups);
}

void ROX_INTERNAL configuration_free(Configuration *configuration) {
    assert(configuration);
    free(configuration->signature_date);
    list_destroy_cb(configuration->experiments, (void (*)(void *)) &experiment_model_free);
    list_destroy_cb(configuration->target_groups, (void (*)(void *)) &target_group_model_free);
    free(configuration);
}

//
// ConfigurationFetchResult
//

const char *ROX_INTERNAL configuration_source_to_str(ConfigurationSource source) {
    assert(source);
    switch (source) {
        case CONFIGURATION_SOURCE_API:
            return "API";
        case CONFIGURATION_SOURCE_CDN:
            return "CDN";
        case CONFIGURATION_SOURCE_ROXY:
            return "ROXY";
        case CONFIGURATION_SOURCE_URL:
            return "URL";
    }
    return "UNKNOWN";
}

ConfigurationFetchResult *ROX_INTERNAL configuration_fetch_result_create(cJSON *data, ConfigurationSource source) {
    assert(data);
    assert(source);
    ConfigurationFetchResult *result = calloc(1, sizeof(ConfigurationFetchResult));
    result->parsed_data = data;
    result->source = source;
    return result;
}

void ROX_INTERNAL configuration_fetch_result_free(ConfigurationFetchResult *result) {
    assert(result);
    if (result->parsed_data) {
        cJSON_Delete(result->parsed_data);
    }
    free(result);
}

//
// RoxConfigurationFetchedArgs
//

RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create(
        RoxFetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes) {
    assert(fetcher_status);
    assert(creation_date);
    RoxConfigurationFetchedArgs *args = calloc(1, sizeof(RoxConfigurationFetchedArgs));
    args->fetcher_status = fetcher_status;
    args->creation_date = creation_date;
    args->has_changes = has_changes;
    args->error_details = NoError;
    return args;
}

RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create_error(RoxFetcherError error_details) {
    assert(error_details);
    RoxConfigurationFetchedArgs *args = calloc(1, sizeof(RoxConfigurationFetchedArgs));
    args->fetcher_status = ErrorFetchedFailed;
    args->error_details = error_details;
    return args;
}

RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_copy(RoxConfigurationFetchedArgs *args) {
    assert(args);
    RoxConfigurationFetchedArgs *copy = calloc(1, sizeof(RoxConfigurationFetchedArgs));
    copy->fetcher_status = args->fetcher_status;
    copy->creation_date = args->creation_date;
    copy->has_changes = args->has_changes;
    copy->error_details = args->error_details;
    return copy;
}

void ROX_INTERNAL configuration_fetched_args_free(RoxConfigurationFetchedArgs *args) {
    assert(args);
    free(args);
}

//
// ConfigurationFetchedInvoker
//

struct ROX_INTERNAL ConfigurationFetchedInvoker {
    void *target;
    rox_configuration_fetched_handler handler;
};

ConfigurationFetchedInvoker *ROX_INTERNAL configuration_fetched_invoker_create() {
    ConfigurationFetchedInvoker *invoker = calloc(1, sizeof(ConfigurationFetchedInvoker));
    return invoker;
}

void ROX_INTERNAL configuration_fetched_invoker_invoke(
        ConfigurationFetchedInvoker *invoker,
        RoxFetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes) {
    assert(invoker);
    assert(fetcher_status);
    assert(creation_date);
    if (!invoker->handler) {
        return;
    }
    RoxConfigurationFetchedArgs *args = configuration_fetched_args_create(fetcher_status, creation_date, has_changes);
    invoker->handler(invoker->target, args);
    configuration_fetched_args_free(args);
}

void ROX_INTERNAL configuration_fetched_invoker_invoke_error(
        ConfigurationFetchedInvoker *invoker,
        RoxFetcherError fetcher_error) {
    assert(invoker);
    assert(fetcher_error);
    if (!invoker->handler) {
        return;
    }
    RoxConfigurationFetchedArgs *args = configuration_fetched_args_create_error(fetcher_error);
    invoker->handler(invoker->target, args);
    configuration_fetched_args_free(args);
}

void ROX_INTERNAL configuration_fetched_invoker_register_handler(
        ConfigurationFetchedInvoker *invoker,
        void *target,
        rox_configuration_fetched_handler handler) {
    assert(invoker);
    assert(handler);
    invoker->target = target;
    invoker->handler = handler;
}

void ROX_INTERNAL configuration_fetched_invoker_free(ConfigurationFetchedInvoker *invoker) {
    assert(invoker);
    free(invoker);
}

//
// ConfigurationParser
//

struct ROX_INTERNAL ConfigurationParser {
    SignatureVerifier *signature_verifier;
    ErrorReporter *error_reporter;
    APIKeyVerifier *api_key_verifier;
    ConfigurationFetchedInvoker *configuration_fetched_invoker;
};

ConfigurationParser *ROX_INTERNAL configuration_parser_create(
        SignatureVerifier *signature_verifier,
        ErrorReporter *error_reporter,
        APIKeyVerifier *api_key_verifier,
        ConfigurationFetchedInvoker *configuration_fetched_invoker) {
    assert(signature_verifier);
    assert(error_reporter);
    assert(api_key_verifier);
    assert(configuration_fetched_invoker);
    ConfigurationParser *parser = calloc(1, sizeof(ConfigurationParser));
    parser->signature_verifier = signature_verifier;
    parser->error_reporter = error_reporter;
    parser->api_key_verifier = api_key_verifier;
    parser->configuration_fetched_invoker = configuration_fetched_invoker;
    return parser;
}

static bool _configuration_parser_is_verified_signature(ConfigurationParser *parser, const cJSON *json) {
    assert(parser);
    assert(json);

    cJSON *data_json = cJSON_GetObjectItem(json, "data");
    if (!data_json || str_is_empty(data_json->valuestring)) {
        error_reporter_report(parser->error_reporter, __FILE__, __LINE__,
                              "Failed to validate signature. Data is empty");
        return false;
    }

    cJSON *signature_v0_json = cJSON_GetObjectItem(json, "signature_v0");
    if (!signature_v0_json || str_is_empty(signature_v0_json->valuestring)) {
        error_reporter_report(parser->error_reporter, __FILE__, __LINE__,
                              "Failed to validate signature. Signature is empty");
        return false;
    }

    if (!signature_verifier_verify(
            parser->signature_verifier,
            data_json->valuestring,
            signature_v0_json->valuestring)) {

        error_reporter_report(
                parser->error_reporter,
                __FILE__, __LINE__,
                "Failed to validate signature. Data : %s Signature : %s",
                data_json->valuestring,
                signature_v0_json->valuestring);

        return false;
    }

    return true;
}

static bool _configuration_parser_is_api_key_verified(ConfigurationParser *parser, cJSON *json) {
    assert(parser);
    assert(json);

    cJSON *application_json = cJSON_GetObjectItem(json, "application");
    if (!application_json || str_is_empty(application_json->valuestring)) {

        error_reporter_report(
                parser->error_reporter,
                __FILE__, __LINE__,
                "Failed to verify app key: \"application\" property is empty");

        return false;
    }

    if (!api_key_verifier_verify(
            parser->api_key_verifier,
            application_json->valuestring)) {

        error_reporter_report(
                parser->error_reporter,
                __FILE__, __LINE__,
                "Failed to parse JSON configuration - Internal Data: %s; SdkSettings: %s",
                application_json->valuestring,
                api_key_verifier_get_sdk_settings(parser->api_key_verifier)->api_key);

        return false;
    }

    return true;
}

static List *_configuration_parser_parse_experiments(ConfigurationParser *parser, cJSON *json) {
    assert(parser);
    assert(json);

    cJSON *experiments_json = cJSON_GetObjectItem(json, "experiments");
    if (!cJSON_IsArray(experiments_json)) {
        return NULL;
    }

    List *result;
    list_new(&result);
    for (int i = 0, n = cJSON_GetArraySize(experiments_json); i < n; ++i) {
        cJSON *exp_json = cJSON_GetArrayItem(experiments_json, i);
        cJSON *deployment_configuration_json = cJSON_GetObjectItem(exp_json, "deploymentConfiguration");
        cJSON *condition_json = cJSON_GetObjectItem(deployment_configuration_json, "condition");
        cJSON *archived_json = cJSON_GetObjectItem(exp_json, "archived");
        cJSON *name_json = cJSON_GetObjectItem(exp_json, "name");
        cJSON *id_json = cJSON_GetObjectItem(exp_json, "_id");
        cJSON *labels_json = cJSON_GetObjectItem(exp_json, "labels");
        cJSON *feature_flags_json = cJSON_GetObjectItem(exp_json, "featureFlags");
        cJSON *stickiness_property_json = cJSON_GetObjectItem(exp_json, "stickinessProperty");

        if (!id_json || !name_json || !condition_json ||
            str_is_empty(id_json->valuestring) ||
            str_is_empty(name_json->valuestring) ||
            str_is_empty(condition_json->valuestring)) {

            error_reporter_report(
                    parser->error_reporter,
                    __FILE__, __LINE__,
                    "Failed to parse configuration: one of \"_id\", \"name\", or "
                    "\"deploymentConfiguration\".\"condition\" is empty");

            list_destroy_cb(result, (void (*)(void *)) &experiment_model_free);
            return NULL;
        }

        HashSet *labels;
        hashset_new(&labels);
        if (labels_json) {
            for (int j = 0, k = cJSON_GetArraySize(labels_json); j < k; ++j) {
                cJSON *label_json = cJSON_GetArrayItem(labels_json, j);
                if (label_json && !str_is_empty(label_json->valuestring)) {
                    hashset_add(labels, label_json->valuestring);
                }
            }
        }

        List *flags;
        list_new(&flags);
        if (feature_flags_json) {
            for (int j = 0, k = cJSON_GetArraySize(feature_flags_json); j < k; ++j) {
                cJSON *flag_json = cJSON_GetArrayItem(feature_flags_json, j);
                if (flag_json) {
                    cJSON *flag_name_json = cJSON_GetObjectItem(flag_json, "name");
                    if (flag_name_json && !str_is_empty(flag_name_json->valuestring)) {
                        list_add(flags, flag_name_json->valuestring);
                    }
                }
            }
        }

        ExperimentModel *model = experiment_model_create(
                id_json->valuestring,
                name_json->valuestring,
                condition_json->valuestring,
                archived_json ? archived_json->valueint : 0,
                flags,
                labels,
                stickiness_property_json ? stickiness_property_json->valuestring : NULL);

        list_add(result, model);
    }

    return result;
}

static List *_configuration_parser_parse_target_groups(ConfigurationParser *parser, cJSON *json) {
    assert(parser);
    assert(json);

    cJSON *target_groups_json = cJSON_GetObjectItem(json, "targetGroups");
    if (!cJSON_IsArray(target_groups_json)) {
        return NULL;
    }

    List *result;
    list_new(&result);
    for (int i = 0, n = cJSON_GetArraySize(target_groups_json); i < n; ++i) {
        cJSON *group_json = cJSON_GetArrayItem(target_groups_json, i);
        cJSON *id_json = cJSON_GetObjectItem(group_json, "_id");
        cJSON *condition_json = cJSON_GetObjectItem(group_json, "condition");

        if (!id_json || !condition_json ||
            str_is_empty(id_json->valuestring) ||
            str_is_empty(condition_json->valuestring)) {
            ROX_ERROR("Invalid JSON provided: no id or condition: %s, %s", id_json, condition_json);
            list_destroy_cb(result, (void (*)(void *)) &target_group_model_free);
            return NULL;
        }

        TargetGroupModel *model = target_group_model_create(
                id_json->valuestring,
                condition_json->valuestring);

        list_add(result, model);
    }

    return result;
}

Configuration *ROX_INTERNAL configuration_parser_parse(
        ConfigurationParser *parser,
        ConfigurationFetchResult *fetch_result) {

    assert(parser);
    assert(fetch_result);

    cJSON *json = fetch_result->parsed_data;
    if (!json) {
        return NULL;
    }

    cJSON *data_json = cJSON_GetObjectItem(json, "data");
    cJSON *signature_date_json = cJSON_GetObjectItem(json, "signed_date");
    if (!signature_date_json || !data_json ||
        str_is_empty(signature_date_json->valuestring) ||
        str_is_empty(data_json->valuestring)) {
        configuration_fetched_invoker_invoke_error(
                parser->configuration_fetched_invoker,
                UnknownError);
        error_reporter_report(parser->error_reporter, __FILE__, __LINE__,
                              "Failed to parse JSON configuration - ",
                              signature_date_json, data_json);
        return NULL;
    }

    if (!_configuration_parser_is_verified_signature(parser, json)) {
        configuration_fetched_invoker_invoke_error(
                parser->configuration_fetched_invoker,
                SignatureVerificationError);
        return NULL;
    }

    cJSON *internal_data_object = cJSON_Parse(data_json->valuestring);
    if (!_configuration_parser_is_api_key_verified(parser, internal_data_object)) {
        configuration_fetched_invoker_invoke_error(
                parser->configuration_fetched_invoker,
                MismatchAppKey);
        return NULL;
    }

    List *experiments = _configuration_parser_parse_experiments(parser, internal_data_object);
    List *target_groups = _configuration_parser_parse_target_groups(parser, internal_data_object);
    if (experiments == NULL || target_groups == NULL) {
        if (experiments) {
            list_destroy_cb(experiments, (void (*)(void *)) &experiment_model_free);
        }
        if (target_groups) {
            list_destroy_cb(target_groups, (void (*)(void *)) &target_group_model_free);
        }
        ROX_ERROR("Failed to parse configurations");
        configuration_fetched_invoker_invoke_error(
                parser->configuration_fetched_invoker,
                UnknownError);
    }

    return configuration_create(
            experiments,
            target_groups,
            signature_date_json->valuestring);
}

void ROX_INTERNAL configuration_parser_free(ConfigurationParser *parser) {
    assert(parser);
    free(parser);
}
