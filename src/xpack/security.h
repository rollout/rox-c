#pragma once

#include "rollout.h"
#include "core/client.h"
#include "core/security.h"

//
// APIKeyVerifier
//

/**
 * @param verifier Not <code>NULL</code>.
 * @return  Not <code>NULL</code>.
 */
ROX_INTERNAL SdkSettings *api_key_verifier_get_sdk_settings(APIKeyVerifier *verifier);
