#pragma once

#include <collectc/list.h>
#include "roxapi.h"
#include "security.h"
#include "reporting.h"
#include "network.h"
#include "client.h"

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
 * param configuration Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_free(Configuration *configuration);

//
// ConfigurationFetchedArgs
//

typedef enum ROX_INTERNAL FetchStatus {
    AppliedFromEmbedded = 1,
    AppliedFromLocalStorage,
    AppliedFromNetwork,
    ErrorFetchedFailed
} FetchStatus;

typedef enum ROX_INTERNAL FetcherError {
    CorruptedJson = 1,
    EmptyJson,
    SignatureVerificationError,
    NetworkError,
    MismatchAppKey,
    Unknown,
    NoError
} FetcherError;

typedef struct ROX_INTERNAL ConfigurationFetchedArgs {
    FetchStatus fetcher_status;
    const char *creation_date;
    bool has_changes;
    FetcherError error_details;
} ConfigurationFetchedArgs;

/**
 * @param creation_date Not <code>null</code>. The caller holds an ownership.
 * @return Not <code>NULL</code>
 */
ConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create(
        FetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes);

ConfigurationFetchedArgs *ROX_INTERNAL configuration_fetched_args_create_error(FetcherError error_details);

void ROX_INTERNAL configuration_fetched_args_free(ConfigurationFetchedArgs *args);

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
        FetchStatus fetcher_status,
        const char *creation_date,
        bool has_changes);

/**
 * @param invoker Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetched_invoker_invoke_error(
        ConfigurationFetchedInvoker *invoker,
        FetcherError fetcher_error);

typedef void ROX_INTERNAL (*configuration_fetched_handler)(void *target, ConfigurationFetchedArgs *args);

/**
 * @param invoker Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
void ROX_INTERNAL configuration_fetched_invoker_register_handler(
        ConfigurationFetchedInvoker *invoker,
        void *target,
        configuration_fetched_handler handler);

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
