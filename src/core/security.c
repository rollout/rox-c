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
