#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <assert.h>
#include "security.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

//
// SignatureVerifier
//

struct SignatureVerifier {
    void *target;
    signature_verifier_func verify;
};

static const char *ROX_CERTIFICATE_BASE64 = "MIIDWDCCAkACCQDR039HDUMyzTANBgkqhkiG9w0BAQUFADBuMQswCQYDVQQHEwJjYTETMBEGA1UEChMKcm9sbG91dC5pbzERMA8GA1UECxMIc2VjdXJpdHkxFzAVBgNVBAMTDnd3dy5yb2xsb3V0LmlvMR4wHAYJKoZIhvcNAQkBFg9leWFsQHJvbGxvdXQuaW8wHhcNMTQwODE4MDkzNjAyWhcNMjQwODE1MDkzNjAyWjBuMQswCQYDVQQHEwJjYTETMBEGA1UEChMKcm9sbG91dC5pbzERMA8GA1UECxMIc2VjdXJpdHkxFzAVBgNVBAMTDnd3dy5yb2xsb3V0LmlvMR4wHAYJKoZIhvcNAQkBFg9leWFsQHJvbGxvdXQuaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDq8GMRFLyaQVDEdcHlYm7NnGrAqhLP2E/27W21yTQein7r8FOT/7jJ0PLpcGLw/3zDT5wzIJ3OtFy4HWre2hn7wmt+bI+bbS/9kKrmqkpjAj1+PwnB4lhEad27lolMCuz5purqi209k7q51IMdfq0/Ot7P/Bmp+LBNs2F4jMsPYxZUUYkVTAmPqgnwxuWoJZan/OGNjtj9OGg8eOcOfcyxC4GDR/Yail+kht4I/HHesSXVukqXntsbdgnXKFkX682TuFPc3pd8ly+6N6OSWpbNV8UmEVZygnxWT3vxBT2TWvFexbW52KOFY91wIkjt+IPEMPJBPPDiN9J2nuttvfMpAgMBAAEwDQYJKoZIhvcNAQEFBQADggEBAIXrD6YsIhZa6fYDAR8huP0V3BRwMKjeLGLCXLzvuPaoQGDhn4RJNgz3leNcomIkV/AwneeS9BXgBAcEKjNeLD+nW58RSRnAfxDT5cUtQgIeR6dFmEK05u+8j/cK3VO410xr0taNMbmJfEn07WjfCdcJS3hsGJuVmEUC85KYznbIcafQMGklLYArXYVnR3XKqzxcLohSPX99weujH5wt78Zy3pXxuYCDETwhgcCYCQaZz7mpvtSOub3JQT+Ir5cBSdyI1oPI2dIamUL5+ntTyll/1rbYj83qREw8PKA9Q0KIIgfpggy19TS9zknwOLz44wRdLyT2tFoaiRqHvm6JKaA=";

static bool xpack_signature_verifier(
        void *target,
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64) {

    assert(verifier);
    assert(data);
    assert(signature_base64);

    char *pem_cert_string = mem_str_format(
            "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----",
            ROX_CERTIFICATE_BASE64);
    size_t pem_cert_len = strlen(pem_cert_string);
    BIO *cert_bio = BIO_new(BIO_s_mem());
    BIO_write(cert_bio, pem_cert_string, pem_cert_len);
    free(pem_cert_string);

    bool verified = false;
    X509 *cert_x509 = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
    BIO_free(cert_bio);

    if (cert_x509) {
        EVP_PKEY *pub_key = X509_get_pubkey(cert_x509);
        EVP_MD_CTX *ctx = EVP_MD_CTX_create();
        if (ctx) {
            if (EVP_VerifyInit(ctx, EVP_sha256())) {
                if (EVP_VerifyUpdate(ctx, data, strlen(data))) {
                    unsigned char sig[256];
                    size_t sig_len = base64_decode_b(signature_base64, sig, sizeof(sig));
                    assert(sig_len == 256);
                    int error_code = EVP_VerifyFinal(ctx, sig, sig_len, pub_key);
                    verified = error_code == 1;
                }
            }
            EVP_MD_CTX_free(ctx);
        }
        EVP_PKEY_free(pub_key);
        X509_free(cert_x509);
    }

    return verified;
}

ROX_INTERNAL SignatureVerifier *signature_verifier_create(SignatureVerifierConfig *config) {
    SignatureVerifier *verifier = calloc(1, sizeof(SignatureVerifier));
    verifier->verify = (config && config->verify_func) ? config->verify_func : &xpack_signature_verifier;
    return verifier;
}

ROX_INTERNAL bool signature_verifier_verify(
        SignatureVerifier *verifier,
        const char *data,
        const char *signature_base64) {
    assert(verifier);
    assert(verifier->verify);
    assert(data);
    assert(signature_base64);
    return verifier->verify(verifier->target, verifier, data, signature_base64);
}

ROX_INTERNAL void signature_verifier_free(SignatureVerifier *verifier) {
    assert(verifier);
    free(verifier);
}

//
// APIKeyVerifier
//

typedef struct APIKeyVerifier {
    SdkSettings *sdk_settings;
    api_key_verifier_func verify;
} APIKeyVerifier;

ROX_INTERNAL bool _api_key_verifier_verify(APIKeyVerifier *key_verifier, const char *api_key) {
    assert(key_verifier);
    assert(api_key);
    return str_equals(key_verifier->sdk_settings->api_key, api_key);
}

ROX_INTERNAL APIKeyVerifier *api_key_verifier_create(APIKeyVerifierConfig *config) {
    assert(config);
    APIKeyVerifier *verifier = calloc(1, sizeof(APIKeyVerifier));
    verifier->sdk_settings = config->payload;
    verifier->verify = config->verify_func ? config->verify_func : &_api_key_verifier_verify;
    return verifier;
}

ROX_INTERNAL bool api_key_verifier_verify(APIKeyVerifier *verifier, const char *api_key) {
    assert(verifier);
    assert(verifier->verify);
    assert(api_key);
    return verifier->verify(verifier, api_key);
}

ROX_INTERNAL SdkSettings *api_key_verifier_get_sdk_settings(APIKeyVerifier *verifier) {
    assert(verifier);
    return verifier->sdk_settings;
}

ROX_INTERNAL void api_key_verifier_free(APIKeyVerifier *verifier) {
    assert(verifier);
    free(verifier);
}
