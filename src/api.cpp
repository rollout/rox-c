#include <cassert>
#include <vector>
#include <string>

#include "rollout.hpp"

extern "C" {
#include "core/client.h"
#include "core/logging.h"
#include "collections.h"
}

namespace Rox {

    //
    // Logging
    //

    void Logging::SetLogLevel(LogLevel logLevel) {
        assert(logLevel >= RoxLogLevelDebug);
        Logging instance = _GetInstance();
        instance._config.min_level = logLevel;
        rox_logging_init(&instance._config);
    }

    static void _RoxLoggingHandlerAdapter(void *target, LogMessage *message) {
        assert(target);
        assert(message);
        auto *handler = (LogMessageHandlerInterface *) target;
        handler->HandleLogMessage(message);
    }

    void Logging::SetLogMessageHandler(LogMessageHandlerInterface *handler) {
        assert(handler);
        Logging instance = _GetInstance();
        instance._config.target = handler;
        instance._config.handler = &_RoxLoggingHandlerAdapter;
        rox_logging_init(&instance._config);
    }

    //
    // Context
    //

    ContextBuilder::ContextBuilder() {
        _map = ROX_EMPTY_MAP;
    }

    ContextBuilder &ContextBuilder::AddBoolValue(const char *name, bool value) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), rox_dynamic_value_create_boolean(value));
        return *this;
    }

    ContextBuilder &ContextBuilder::AddIntValue(const char *name, int value) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), rox_dynamic_value_create_int(value));
        return *this;
    }

    ContextBuilder &ContextBuilder::AddDoubleValue(const char *name, double value) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), rox_dynamic_value_create_double(value));
        return *this;
    }

    ContextBuilder &ContextBuilder::AddStringValue(const char *name, const char *value) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), value
                                              ? rox_dynamic_value_create_string_copy(value)
                                              : rox_dynamic_value_create_null());
        return *this;
    }

    ContextBuilder &ContextBuilder::AddUndefined(const char *name) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), rox_dynamic_value_create_undefined());
        return *this;
    }

    ContextBuilder &ContextBuilder::AddNull(const char *name) {
        assert(name);
        rox_map_add(_map, mem_copy_str(name), rox_dynamic_value_create_null());
        return *this;
    }

    Context *ContextBuilder::Build() {
        return rox_context_create_from_map(_map);
    }

    //
    // Options
    //

    static void _RoxImpressionHandlerAdapter(
            void *target,
            RoxReportingValue *value,
            RoxExperiment *experiment,
            RoxContext *context) {
        assert(target);
        auto *handler = (ImpressionHandlerInterface *) target;
        handler->HandleImpression(value, experiment, context);
    }

    static void _RoxConfigurationFetchedHandlerAdapter(void *target, RoxConfigurationFetchedArgs *args) {
        assert(target);
        auto *handler = (ConfigurationFetchedHandler *) target;
        handler->ConfigurationFetched(args);
    }

    static RoxDynamicValue *_RoxDynamicPropertiesRuleAdapter(
            const char *prop_name,
            void *target,
            RoxContext *context) {
        assert(prop_name);
        assert(target);
        auto rule = (DynamicPropertiesRuleInterface *) target;
        return rule->Invoke(prop_name, context);
    }

    OptionsBuilder::OptionsBuilder() {
        _options = rox_options_create();
    }

    OptionsBuilder &OptionsBuilder::SetDevModeKey(const char *devModeKey) {
        assert(devModeKey);
        rox_options_set_dev_mode_key(_options, devModeKey);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetVersion(const char *version) {
        assert(version);
        rox_options_set_version(_options, version);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetFetchInterval(int intervalInSeconds) {
        assert(intervalInSeconds >= 0);
        rox_options_set_fetch_interval(_options, intervalInSeconds);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetRoxyUrl(const char *roxy_url) {
        assert(roxy_url);
        rox_options_set_roxy_url(_options, roxy_url);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetImpressionHandler(ImpressionHandlerInterface *handler) {
        assert(handler);
        rox_options_set_impression_handler(_options, handler, &_RoxImpressionHandlerAdapter);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetConfigurationFetchedHandler(ConfigurationFetchedHandler *handler) {
        assert(handler);
        rox_options_set_configuration_fetched_handler(_options, handler,
                                                      &_RoxConfigurationFetchedHandlerAdapter);
        return *this;
    }

    ROX_API OptionsBuilder &OptionsBuilder::SetDynamicPropertiesRule(DynamicPropertiesRuleInterface *rule) {
        assert(rule);
        rox_options_set_dynamic_properties_rule(_options, rule, &_RoxDynamicPropertiesRuleAdapter);
        return *this;
    }

    ROX_API Options *OptionsBuilder::Build() {
        return _options;
    }

    //
    // Flags
    //

    RoxList *Variant::_allVariants = ROX_EMPTY_LIST;

    ROX_API Variant *Variant::Create(const char *name, const char *defaultValue) {
        assert(name);
        auto *variant = new Variant(rox_add_variant(name, defaultValue, nullptr));
        rox_list_add(_allVariants, variant);
        return variant;
    }

    ROX_API Variant *
    Variant::Create(const char *name, const char *defaultValue, const std::vector<std::string> &options) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto &option : options) {
            rox_list_add(list, ROX_COPY(option.data()));
        }
        auto *variant = new Variant(rox_add_variant(name, defaultValue, list));
        rox_list_add(_allVariants, variant);
        return variant;
    }

    ROX_API char *Variant::GetValue(Context *context) {
        return (context == nullptr)
               ? rox_variant_get_value_or_default(_variant)
               : rox_variant_get_value_or_default_ctx(_variant, context);
    }

    ROX_API char *Variant::GetValueOrNull(Context *context) {
        return (context == nullptr)
               ? rox_variant_get_value_or_null(_variant)
               : rox_variant_get_value_or_null_ctx(_variant, context);
    }

    ROX_API Flag *Flag::Create(const char *name, bool defaultValue) {
        assert(name);
        RoxVariant *handle = rox_add_flag(name, defaultValue);
        auto *flag = new Flag(handle);
        rox_list_add(_allVariants, flag);
        return flag;
    }

    ROX_API bool Flag::IsEnabled(Context *context) {
        return context == nullptr
               ? rox_flag_is_enabled(_variant)
               : rox_flag_is_enabled_ctx(_variant, context);
    }

    ROX_API bool *Flag::IsEnabledOrNull(Context *context) {
        return const_cast<bool *>(
                (context == nullptr)
                ? rox_flag_is_enabled_or_null(_variant)
                : rox_flag_is_enabled_or_null_ctx(_variant, context));
    }

    //
    // CustomProperties
    //

    void _WarnUnsupportedCustomPropertyValueType() {
        ROX_ERROR(
                "Calling SetCustomProperty with an unsupported value type. "
                "Only int, double, bool, and const char* types are supported "
                "for custom property values.");
    }

    template<typename T>
    ROX_API void SetCustomProperty(const char *name, T value) {
        assert(name);
        _WarnUnsupportedCustomPropertyValueType();
    }

    template<typename T>
    ROX_API void SetCustomComputedProperty(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        _WarnUnsupportedCustomPropertyValueType();
    }

    template<>
    ROX_API void SetCustomProperty<bool>(const char *name, bool value) {
        assert(name);
        rox_set_custom_boolean_property(name, value);
    }

    template<>
    ROX_API void SetCustomProperty<int>(const char *name, int value) {
        assert(name);
        rox_set_custom_integer_property(name, value);
    }

    template<>
    ROX_API void SetCustomProperty<double>(const char *name, double value) {
        assert(name);
        rox_set_custom_double_property(name, value);
    }

    template<>
    ROX_API void SetCustomProperty<const char *>(const char *name, const char *value) {
        assert(name);
        rox_set_custom_string_property(name, value);
    }

    ROX_API void SetCustomSemverProperty(const char *name, const char *value) {
        assert(name);
        rox_set_custom_semver_property(name, value);
    }

    RoxDynamicValue *_PropertyValueGeneratorAdapter(void *target, RoxContext *context) {
        assert(target);
        auto generator = (CustomPropertyGeneratorInterface *) target;
        return generator->operator()(context);
    }

    template<>
    ROX_API void SetCustomComputedProperty<bool>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_boolean_property(name, generator, &_PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void SetCustomComputedProperty<int>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_integer_property(name, generator, &_PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void SetCustomComputedProperty<double>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_double_property(name, generator, &_PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void
    SetCustomComputedProperty<const char *>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_string_property(name, generator, &_PropertyValueGeneratorAdapter);
    }

    ROX_API void SetCustomComputedSemverProperty(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_semver_property(name, generator, &_PropertyValueGeneratorAdapter);
    }

    //
    // Setup/Shutdown
    //

    ROX_API void Setup(const char *api_key, Options *options) {
        assert(api_key);
        assert(options);
        rox_setup(api_key, options);
    }

    ROX_API void Shutdown() {
        rox_shutdown();
        if (Variant::_allVariants) {
            ROX_LIST_FOREACH(item, Variant::_allVariants, {
                delete (Variant *) item;
            })
            rox_list_free(Variant::_allVariants);
            Variant::_allVariants = ROX_EMPTY_LIST;
        }
    }

    //
    // Misc
    //

    ROX_API void SetContext(Context *context) {
        assert(context);
        rox_set_context(context);
    }

    ROX_API void Fetch() {
        rox_fetch();
    }

    //
    // Dynamic API
    //

    ROX_API DynamicApi::~DynamicApi() {
        rox_dynamic_api_free(_handle);
    }

    ROX_API DynamicApi *DynamicApi::Create() {
        return new DynamicApi(rox_dynamic_api());
    }

    ROX_API bool DynamicApi::IsEnabled(const char *name,
                                       bool default_value,
                                       Context *context) {
        assert(name);
        return rox_dynamic_api_is_enabled(_handle, name, default_value, context);
    }

    ROX_API char *DynamicApi::GetValue(const char *name,
                                       char *default_value,
                                       Context *context) {
        assert(name);
        return rox_dynamic_api_get_value(_handle, name, default_value, nullptr, context);
    }

    ROX_API char *DynamicApi::GetValue(const char *name,
                                       char *default_value,
                                       const std::vector<std::string> &options,
                                       Context *context) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto &option : options) {
            rox_list_add(list, ROX_COPY(option.data()));
        }
        return rox_dynamic_api_get_value(_handle, name, default_value, list, context);
    }
}
