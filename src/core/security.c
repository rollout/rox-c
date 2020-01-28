#include <stdlib.h>
#include <assert.h>
#include "security.h"

//
// SignatureVerifier
//

struct ROX_INTERNAL SignatureVerifier {
    signature_verifier_func verify;
};

bool ROX_INTERNAL _signature_verifier_verify(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64) {
    assert(verifier);
    assert(data);
    assert(signature_base64);
    // TODO: port from X-Pack
    return true;
}

SignatureVerifier *ROX_INTERNAL signature_verifier_create(SignatureVerifierConfig *config) {
    assert(config);
    SignatureVerifier *verifier = calloc(1, sizeof(SignatureVerifier));
    verifier->verify = config->verify_func ? config->verify_func : &_signature_verifier_verify;
    return verifier;
}

bool ROX_INTERNAL signature_verifier_verify(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64) {
    assert(verifier);
    assert(verifier->verify);
    assert(data);
    assert(signature_base64);
    return verifier->verify(verifier, data, signature_base64);
}

void ROX_INTERNAL signature_verifier_free(SignatureVerifier *verifier) {
    assert(verifier);
    free(verifier);
}

//
// APIKeyVerifier
//

typedef struct ROX_INTERNAL APIKeyVerifier {
    SdkSettings *sdk_settings;
    api_key_verifier_func verify;
} APIKeyVerifier;

bool ROX_INTERNAL _api_key_verifier_verify(APIKeyVerifier *key_verifier, const char *api_key) {
    assert(key_verifier);
    assert(api_key);
    // TODO: port from X-Pack
    return true;
}

APIKeyVerifier *ROX_INTERNAL api_key_verifier_create(APIKeyVerifierConfig *config) {
    assert(config);
    APIKeyVerifier *verifier = calloc(1, sizeof(APIKeyVerifier));
    verifier->sdk_settings = config->sdk_settings;
    verifier->verify = config->verify_func ? config->verify_func : &_api_key_verifier_verify;
    return verifier;
}

bool ROX_INTERNAL api_key_verifier_verify(APIKeyVerifier *verifier, const char *api_key) {
    assert(verifier);
    assert(verifier->verify);
    assert(api_key);
    return verifier->verify(verifier, api_key);
}

SdkSettings *ROX_INTERNAL api_key_verifier_get_sdk_settings(APIKeyVerifier *verifier) {
    assert(verifier);
    return verifier->sdk_settings;
}

void ROX_INTERNAL api_key_verifier_free(APIKeyVerifier *verifier) {
    assert(verifier);
    free(verifier);
}
