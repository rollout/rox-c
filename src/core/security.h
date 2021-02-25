#pragma once

#include <stdbool.h>
#include "rox/server.h"

//
// SignatureVerifier
//

typedef struct SignatureVerifier SignatureVerifier;

typedef bool (*signature_verifier_func)(
        void *target,
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64);

typedef struct SignatureVerifierConfig {
    void *target;
    signature_verifier_func verify_func;
} SignatureVerifierConfig;

/**
 * @param config May be <code>NULL</code>.
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL SignatureVerifier *signature_verifier_create(SignatureVerifierConfig *config);

/**
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL SignatureVerifier *signature_verifier_create_dummy();

/**
 * @param verifier Not <code>NULL</code>.
 * @param data Not <code>NULL</code>.
 * @param signature_base64 Not <code>NULL</code>.
 */
ROX_INTERNAL bool signature_verifier_verify(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64);

/**
 * @param verifier Not <code>NULL</code>.
 */
ROX_INTERNAL void signature_verifier_free(SignatureVerifier *verifier);

//
// APIKeyVerifier
//

typedef struct APIKeyVerifier APIKeyVerifier;

typedef bool (*api_key_verifier_func)(APIKeyVerifier *key_verifier, const char *api_key);

typedef struct APIKeyVerifierConfig {
    void *payload;
    api_key_verifier_func verify_func;
} APIKeyVerifierConfig;

/**
 * @param config Not <code>NULL</code>
 * @return Not <code>NULL</code>.
 */
ROX_INTERNAL APIKeyVerifier *api_key_verifier_create(APIKeyVerifierConfig *config);

ROX_INTERNAL APIKeyVerifier *api_key_verifier_create_dummy();

/**
 * @param verifier Not <code>NULL</code>.
 * @param api_key Not <code>NULL</code>.
 */
ROX_INTERNAL bool api_key_verifier_verify(APIKeyVerifier *verifier, const char *api_key);

/**
 * @param verifier Not <code>NULL</code>.
 */
ROX_INTERNAL void api_key_verifier_free(APIKeyVerifier *verifier);
