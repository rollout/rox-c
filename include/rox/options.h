#pragma once

#include <rox/defs.h>
#include <rox/values.h>
#include <rox/impression.h>
#include <rox/configuration.h>

/**
 * Note the returned value, if not <code>NULL</code>, must be freed by the caller
 * by invoking <code>dynamic_value_free()</code>
 */
typedef RoxDynamicValue *(*rox_dynamic_properties_rule)(
        const char *prop_name,
        void *target,
        RoxContext *context);

typedef struct RoxOptions RoxOptions;

ROX_API RoxOptions *rox_options_create();

/**
 * The caller is responsible for freeing the passed <code>key</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param key Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_dev_mode_key(RoxOptions *options, const char *key);

/**
 * The caller is responsible for freeing the passed <code>version</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param version Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_version(RoxOptions *options, const char *version);

/**
 * @param options Not <code>NULL</code>.
 * @param fetch_interval Interval in seconds. Should be not less than 30. Otherwise, 30 will be used.
 */
ROX_API void rox_options_set_fetch_interval(RoxOptions *options, int fetch_interval);

/**
 * The caller is responsible for freeing the passed <code>roxy_url</code> value after use.
 *
 * @param options Not <code>NULL</code>.
 * @param roxy_url Not <code>NULL</code>. Value is copied internally.
 */
ROX_API void rox_options_set_roxy_url(
        RoxOptions *options,
        const char *roxy_url);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_API void rox_options_set_impression_handler(
        RoxOptions *options,
        void *target,
        rox_impression_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param handler Not <code>NULL</code>.
 */
ROX_API void rox_options_set_configuration_fetched_handler(
        RoxOptions *options,
        void *target,
        rox_configuration_fetched_handler handler);

/**
 * @param options Not <code>NULL</code>.
 * @param target May be <code>NULL</code>.
 * @param rule Not <code>NULL</code>.
 */
ROX_API void rox_options_set_dynamic_properties_rule(
        RoxOptions *options,
        void *target,
        rox_dynamic_properties_rule rule);

/**
 * @param options Not <code>NULL</code>.
 * @param verification_disabled if the system skips checking the signature verification. true, yes (platform preferred); false no
 */
ROX_API void rox_options_set_disable_signature_verification(RoxOptions *options, bool verification_disabled);