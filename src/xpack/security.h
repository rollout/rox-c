#pragma once

#include "rollout.h"
#include "core/client.h"
#include "core/security.h"

//
// APIKeyVerifier
//

typedef bool (*api_key_verifier_func)(APIKeyVerifier *key_verifier, const char *api_key);

typedef struct APIKeyVerifierConfig {
    SdkSettings *sdk_settings;
    api_key_verifier_func verify_func;
} APIKeyVerifierConfig;

/**
 * @param config Not <code>NULL</code>
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL APIKeyVerifier *api_key_verifier_create(APIKeyVerifierConfig *config);

/**
 * @param verifier Not <code>NULL</code>.
 * @return  Not <code>NULL</code>.
 */
ROX_INTERNAL SdkSettings *api_key_verifier_get_sdk_settings(APIKeyVerifier *verifier);
