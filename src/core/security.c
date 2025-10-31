#include <stddef.h>
#include "security.h"

static bool _dummy_signature_verifier_func(
        void *target,
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64) {
    return true;
}

static bool _dummy_api_key_verifier_func(APIKeyVerifier *key_verifier, const char *api_key) {
    return true;
}

ROX_INTERNAL SignatureVerifier *signature_verifier_create_dummy() {
    SignatureVerifierConfig config;
    config.target = NULL;
    config.skip_verification = true;
    config.verify_func = &_dummy_signature_verifier_func;
    return signature_verifier_create(&config);
}

ROX_INTERNAL APIKeyVerifier *api_key_verifier_create_dummy() {
    APIKeyVerifierConfig config;
    config.payload = NULL;
    config.verify_func = &_dummy_api_key_verifier_func;
    return api_key_verifier_create(&config);
}
