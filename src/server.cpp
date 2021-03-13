#include <cassert>
#include <vector>
#include <string>

extern "C" {
#include "core/client.h"
#include "core/logging.h"
#include "collections.h"
#include "util.h"
}

#include <roxx/server.h>

namespace Rox {

    //
    // Logging
    //

    void Logging::SetLogLevel(LogLevel logLevel) {
        assert(logLevel >= RoxLogLevelDebug);
        Logging &instance = GetInstance();
        instance._config.min_level = logLevel;
        rox_logging_init(&instance._config);
    }

    static void RoxLoggingHandlerAdapter(void *target, LogMessage *message) {
        assert(target);
        assert(message);
        auto *handler = (LogMessageHandlerInterface *) target;
        handler->HandleLogMessage(message);
    }

    void Logging::SetLogMessageHandler(LogMessageHandlerInterface *handler) {
        assert(handler);
        Logging &instance = GetInstance();
        instance._config.target = handler;
        instance._config.handler = &RoxLoggingHandlerAdapter;
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

    static void RoxImpressionHandlerAdapter(
            void *target,
            RoxReportingValue *value,
            RoxContext *context) {
        assert(target);
        auto *handler = (ImpressionHandlerInterface *) target;
        handler->HandleImpression(value, context);
    }

    static void RoxConfigurationFetchedHandlerAdapter(void *target, RoxConfigurationFetchedArgs *args) {
        assert(target);
        auto *handler = (ConfigurationFetchedHandlerInterface *) target;
        handler->ConfigurationFetched(args);
    }

    static RoxDynamicValue *RoxDynamicPropertiesRuleAdapter(
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
        rox_options_set_impression_handler(_options, handler, &RoxImpressionHandlerAdapter);
        return *this;
    }

    OptionsBuilder &OptionsBuilder::SetConfigurationFetchedHandler(ConfigurationFetchedHandlerInterface *handler) {
        assert(handler);
        rox_options_set_configuration_fetched_handler(_options, handler,
                                                      &RoxConfigurationFetchedHandlerAdapter);
        return *this;
    }

    ROX_API OptionsBuilder &OptionsBuilder::SetDynamicPropertiesRule(DynamicPropertiesRuleInterface *rule) {
        assert(rule);
        rox_options_set_dynamic_properties_rule(_options, rule, &RoxDynamicPropertiesRuleAdapter);
        return *this;
    }

    ROX_API Options *OptionsBuilder::Build() {
        return _options;
    }

    //
    // Flags
    //

    RoxList *BaseFlag::_allVariants = nullptr;

    ROX_API BaseFlag::BaseFlag(RoxStringBase *variant) : _variant(variant) {
        if (!_allVariants) {
            _allVariants = ROX_EMPTY_LIST;
        }
        rox_list_add(_allVariants, this);
    }

    ROX_API const char *BaseFlag::GetName() {
        return variant_get_name(_variant);
    }

    ROX_API String *String::Create(const char *name, const char *defaultValue) {
        assert(name);
        return new String(rox_add_string(name, defaultValue));
    }

    ROX_API String *
    String::Create(const char *name, const char *defaultValue, const std::vector<std::string> &options) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto &option : options) {
            rox_list_add(list, ROX_COPY(option.data()));
        }
        return new String(rox_add_string_with_options(name, defaultValue, list));
    }

    ROX_API char *String::GetValue(Context *context) {
        return (context == nullptr)
               ? rox_get_string(_variant)
               : rox_get_string_ctx(_variant, context);
    }

    ROX_API Flag *Flag::Create(const char *name, bool defaultValue) {
        assert(name);
        return new Flag(rox_add_flag(name, defaultValue));
    }

    ROX_API bool Flag::IsEnabled(Context *context) {
        return context == nullptr
               ? rox_is_enabled(_variant)
               : rox_is_enabled_ctx(_variant, context);
    }

    ROX_API Int *Int::Create(const char *name, int defaultValue) {
        assert(name);
        return new Int(rox_add_int(name, defaultValue));
    }

    ROX_API Int *Int::Create(const char *name, int defaultValue, const std::vector<int> &options) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto it = options.begin(); it != options.end(); it++) {
            rox_list_add(list, mem_int_to_str(*it));
        }
        return new Int(rox_add_int_with_options(name, defaultValue, list));
    }

    ROX_API int Int::GetValue(Context *context) {
        return (context == nullptr)
               ? rox_get_int(_variant)
               : rox_get_int_ctx(_variant, context);
    }

    ROX_API double Double::GetValue(Context *context) {
        return (context == nullptr)
               ? rox_get_double(_variant)
               : rox_get_double_ctx(_variant, context);
    }

    ROX_API Double *Double::Create(const char *name, double defaultValue) {
        assert(name);
        return new Double(rox_add_double(name, defaultValue));
    }

    ROX_API Double *Double::Create(const char *name, double defaultValue, const std::vector<double> &options) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto it = options.begin(); it != options.end(); it++) {
            rox_list_add(list, mem_double_to_str(*it));
        }
        return new Double(rox_add_double_with_options(name, defaultValue, list));
    }

    //
    // CustomProperties
    //

    void WarnUnsupportedCustomPropertyValueType() {
        ROX_ERROR(
                "Calling SetCustomProperty with an unsupported value type. "
                "Only int, double, bool, and const char* types are supported "
                "for custom property values.");
    }

    template<typename T>
    ROX_API void SetCustomProperty(const char *name, T value) {
        assert(name);
        WarnUnsupportedCustomPropertyValueType();
    }

    template<typename T>
    ROX_API void SetCustomComputedProperty(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        WarnUnsupportedCustomPropertyValueType();
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

    RoxDynamicValue *PropertyValueGeneratorAdapter(void *target, RoxContext *context) {
        assert(target);
        auto generator = (CustomPropertyGeneratorInterface *) target;
        return generator->operator()(context);
    }

    template<>
    ROX_API void SetCustomComputedProperty<bool>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_boolean_property(name, generator, &PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void SetCustomComputedProperty<int>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_integer_property(name, generator, &PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void SetCustomComputedProperty<double>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_double_property(name, generator, &PropertyValueGeneratorAdapter);
    }

    template<>
    ROX_API void
    SetCustomComputedProperty<const char *>(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_string_property(name, generator, &PropertyValueGeneratorAdapter);
    }

    ROX_API void SetCustomComputedSemverProperty(const char *name, CustomPropertyGeneratorInterface *generator) {
        assert(name);
        assert(generator);
        rox_set_custom_computed_semver_property(name, generator, &PropertyValueGeneratorAdapter);
    }

    //
    // Setup/Shutdown
    //

    ROX_API StateCode Setup(const char *api_key, Options *options) {
        assert(api_key);
        if (!options) {
            options = rox_options_create();
        }
        rox_options_set_cxx(options);
        RoxStateCode code = rox_setup(api_key, options);
        if (code < 0) {
            throw SetupException("Rox::Setup failed", code);
        }
        return code;
    }

    ROX_API void Shutdown() {
        rox_shutdown();
        if (BaseFlag::_allVariants) {
            ROX_LIST_FOREACH(item, BaseFlag::_allVariants, {
                delete (BaseFlag *) item;
            })
            rox_list_free(BaseFlag::_allVariants);
            BaseFlag::_allVariants = nullptr;
        }
    }

    //
    // Misc
    //

    ROX_API void SetContext(Context *context) {
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
        return rox_dynamic_api_is_enabled_ctx(_handle, name, default_value, context);
    }

    ROX_API char *DynamicApi::GetString(const char *name,
                                        char *default_value,
                                        Context *context) {
        assert(name);
        return rox_dynamic_api_get_string_ctx(_handle, name, default_value, nullptr, context);
    }

    ROX_API char *DynamicApi::GetString(const char *name,
                                        char *default_value,
                                        const std::vector<std::string> &options,
                                        Context *context) {
        assert(name);
        RoxList *list = ROX_EMPTY_LIST;
        for (auto &option : options) {
            rox_list_add(list, ROX_COPY(option.data()));
        }
        return rox_dynamic_api_get_string_ctx(_handle, name, default_value, list, context);
    }

    ROX_API int DynamicApi::GetInt(const char *name, int default_value, Context *context) {
        return rox_dynamic_api_get_int_ctx(_handle, name, default_value, nullptr, context);
    }

    ROX_API int
    DynamicApi::GetInt(const char *name, int default_value, const std::vector<int> &options, Context *context) {
        RoxList *list = ROX_EMPTY_LIST;
        for (auto it = options.begin(); it != options.end(); it++) {
            rox_list_add(list, mem_int_to_str(*it));
        }
        return rox_dynamic_api_get_int_ctx(_handle, name, default_value, list, context);
    }

    ROX_API double DynamicApi::GetDouble(const char *name, double default_value, Context *context) {
        return rox_dynamic_api_get_double_ctx(_handle, name, default_value, nullptr, context);
    }

    ROX_API double DynamicApi::GetDouble(const char *name, double default_value, const std::vector<double> &options,
                                         Context *context) {
        RoxList *list = ROX_EMPTY_LIST;
        for (auto it = options.begin(); it != options.end(); it++) {
            rox_list_add(list, mem_double_to_str(*it));
        }
        return rox_dynamic_api_get_double_ctx(_handle, name, default_value, list, context);
    }
}
