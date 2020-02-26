#pragma once

#include <collectc/list.h>
#include <cjson/cJSON.h>

#include "rollout.h"
#include "security.h"
#include "reporting.h"

//
// Configuration
//

typedef struct ROX_INTERNAL Configuration {
    char *signature_date;
    List *experiments;
    List *target_groups;
} Configuration;

/**
 * The returned pointer must be freed after use by calling <code>configuration_free()</code>.
 *
 * @param experiments Not <code>NULL</code>. Ownership is delegated to the returned <code>configuration</code>.
 * @param target_groups Not <code>NULL</code>. Ownership is delegated to the returned <code>configuration</code>.
 * @param signature_date Not <code>NULL</code>. Will be copied internally. The caller holds an ownership of the given string.
 * @return Not <code>NULL</code>.
 */
Configuration *ROX_INTERNAL configuration_create(
        List *experiments,
        List *target_groups,
        const char *signature_date);

/**
 * @param c1 Not <code>NULL</code>.
 * @param c2 Not <code>NULL</code>.
 */
bool ROX_INTERNAL configuration_equals(Configuration *c1, Configuration *c2);

/**
 * param configuration Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_free(Configuration *configuration);

//
// ConfigurationFetchResult
//

typedef enum ROX_INTERNAL ConfigurationSource {
    CONFIGURATION_SOURCE_CDN = 1,
    CONFIGURATION_SOURCE_API,
    CONFIGURATION_SOURCE_ROXY,
    CONFIGURATION_SOURCE_URL
} ConfigurationSource;

const char *ROX_INTERNAL configuration_source_to_str(ConfigurationSource source);

typedef struct ROX_INTERNAL ConfigurationFetchResult {
    ConfigurationSource source;
    cJSON *parsed_data;
} ConfigurationFetchResult;

/**
 * @param data Not <code>NULL</code>. The caller is responsible for freeing the given pointer.
 * @return Not <code>NULL</code>.
 */
ConfigurationFetchResult *ROX_INTERNAL configuration_fetch_result_create(cJSON *data, ConfigurationSource source);

/**
 * @param result Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetch_result_free(ConfigurationFetchResult *result);

//
// RoxConfigurationFetchedArgs
//

/**
 * @param creation_date Not <code>null</code>. The caller holds an ownership.
 * @return Not <code>NULL</code>
 */
RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create(
        RoxFetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes);

RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create_error(RoxFetcherError error_details);

RoxConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_copy(RoxConfigurationFetchedArgs *args);

void ROX_INTERNAL configuration_fetched_args_free(RoxConfigurationFetchedArgs *args);

//
// ConfigurationFetchedInvoker
//

typedef struct ROX_INTERNAL ConfigurationFetchedInvoker ConfigurationFetchedInvoker;

void ROX_INTERNAL configuration_fetched_invoker_free(ConfigurationFetchedInvoker *invoker);

ConfigurationFetchedInvoker *ROX_INTERNAL configuration_fetched_invoker_create();

/**
 * @param invoker Not <code>NULL</code>.
 * @param creation_date Unix timestamp with millis.
 */
void ROX_INTERNAL configuration_fetched_invoker_invoke(
        ConfigurationFetchedInvoker *invoker,
        RoxFetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes);

/**
 * @param invoker Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetched_invoker_invoke_error(
        ConfigurationFetchedInvoker *invoker,
        RoxFetcherError fetcher_error);

/**
 * @param invoker Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetched_invoker_register_handler(
        ConfigurationFetchedInvoker *invoker,
        void *target,
        rox_configuration_fetched_handler handler);

//
// ConfigurationParser
//

typedef struct ROX_INTERNAL ConfigurationParser ConfigurationParser;

/**
 * @param signature_verifier Not <code>NULL</code>. The caller holds the ownership on this object.
 * @param error_reporter Not <code>NULL</code>. The caller holds the ownership on this object.
 * @param api_key_verifier Not <code>NULL</code>. The caller holds the ownership on this object.
 * @param configuration_fetched_invoker Not <code>NULL</code>. The caller holds the ownership on this object.
 * @return Not <code>NULL</code>.
 */
ConfigurationParser *ROX_INTERNAL configuration_parser_create(
        SignatureVerifier *signature_verifier,
        ErrorReporter *error_reporter,
        APIKeyVerifier *api_key_verifier,
        ConfigurationFetchedInvoker *configuration_fetched_invoker);

/**
 * @param parser Not <code>NULL</code>.
 * @param fetch_result Not <code>NULL</code>.
 * @return May be <code>NULL</code>.
 */
Configuration *ROX_INTERNAL  configuration_parser_parse(
        ConfigurationParser *parser,
        ConfigurationFetchResult *fetch_result);

/**
 * @param parser Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_parser_free(ConfigurationParser *parser);
