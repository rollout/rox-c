#pragma once

#include <stdbool.h>
#include "roxapi.h"
#include "client.h"

//
// SignatureVerifier
//

typedef struct ROX_INTERNAL SignatureVerifier SignatureVerifier;

typedef bool ROX_INTERNAL (*signature_verifier_func)(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64);

typedef struct ROX_INTERNAL SignatureVerifierConfig {
    signature_verifier_func verify_func;
} SignatureVerifierConfig;

/**
 * @param config Not <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
SignatureVerifier *ROX_INTERNAL signature_verifier_create(SignatureVerifierConfig *config);

/**
 * @param verifier Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @param signature_base64 Not <code>NULL</code>.
 */
bool ROX_INTERNAL signature_verifier_verify(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64);

/**
 * @param verifier Not <code>NULL</code>.
 */
void ROX_INTERNAL signature_verifier_free(SignatureVerifier *verifier);

//
// APIKeyVerifier
//

typedef struct ROX_INTERNAL APIKeyVerifier APIKeyVerifier;

typedef bool ROX_INTERNAL (*api_key_verifier_func)(APIKeyVerifier *key_verifier, const char *api_key);

typedef struct ROX_INTERNAL APIKeyVerifierConfig {
    SdkSettings *sdk_settings;
    api_key_verifier_func verify_func;
} APIKeyVerifierConfig;

/**
 * @param config Not <code>NULL</code>
 * @return Not <code>NULL</code>.
 */
APIKeyVerifier *ROX_INTERNAL api_key_verifier_create(APIKeyVerifierConfig *config);

/**
 * @param verifier Not <code>NULL</code>.
 * @param api_key Not <code>NULL</code>.
 */
bool ROX_INTERNAL api_key_verifier_verify(APIKeyVerifier *verifier, const char *api_key);

/**
 * @param verifier Not <code>NULL</code>.
 * @return  Not <code>NULL</code>.
 */
SdkSettings *ROX_INTERNAL api_key_verifier_get_sdk_settings(APIKeyVerifier *verifier);

/**
 * @param verifier Not <code>NULL</code>.
 */
void ROX_INTERNAL api_key_verifier_free(APIKeyVerifier *verifier);
