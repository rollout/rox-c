#include "security.h"
#include <assert.h>

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
