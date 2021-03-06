#pragma once

#include <stdbool.h>

typedef enum RoxFetchStatus {
    AppliedFromEmbedded = 1,
    AppliedFromLocalStorage,
    AppliedFromNetwork,
    ErrorFetchedFailed
} RoxFetchStatus;

typedef enum RoxFetcherError {
    NoError = 0,
    CorruptedJson = 1,
    EmptyJson,
    SignatureVerificationError,
    NetworkError,
    MismatchAppKey,
    UnknownError
} RoxFetcherError;

typedef struct RoxConfigurationFetchedArgs {
    RoxFetchStatus fetcher_status;
    const char *creation_date;
    bool has_changes;
    RoxFetcherError error_details;
} RoxConfigurationFetchedArgs;

typedef void (*rox_configuration_fetched_handler)(void *target, RoxConfigurationFetchedArgs *args);