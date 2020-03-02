#pragma once

#include <cassert>
#include <vector>
#include <string>

extern "C" {
#include "rollout.h"
}

namespace Rox {

    // Structs

    typedef struct RoxReportingValue ReportingValue;

    typedef struct RoxExperiment Experiment;

    typedef struct RoxConfigurationFetchedArgs ConfigurationFetchedArgs;

    typedef struct RoxLogMessage LogMessage;

    typedef struct RoxDynamicValue DynamicValue;

    typedef struct RoxContext Context;

    typedef struct RoxOptions Options;

    typedef enum RoxLogLevel LogLevel;

    //
    // Logging
    //

    class ROX_API LogMessageHandlerInterface {
    public:
        virtual void HandleLogMessage(LogMessage *message) = 0;

        virtual ~LogMessageHandlerInterface() = default;
    };

    class ROX_API Logging {
    private:
        RoxLoggingConfig _config;

        Logging() : _config(ROX_LOGGING_CONFIG_INITIALIZER(RoxLogLevelError)) {
        }

        // Stop the compiler generating methods of copy the object
        Logging(Logging const &copy) = default;

        Logging &operator=(Logging const &copy) = default;

        static Logging &_GetInstance() {
            // The only instance
            // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
            static Logging instance;
            return instance;
        }

    public:
        static void SetLogLevel(LogLevel logLevel);

        static void SetLogMessageHandler(LogMessageHandlerInterface *handler);
    };

    //
    // Context
    //

    class ROX_API ContextBuilder {
    private:
        RoxMap *_map;

    public:
        ContextBuilder();

        ContextBuilder &AddBoolValue(const char *name, bool value);

        ContextBuilder &AddIntValue(const char *name, int value);

        ContextBuilder &AddDoubleValue(const char *name, double value);

        ContextBuilder &AddStringValue(const char *name, const char *value);

        ContextBuilder &AddUndefined(const char *name);

        ContextBuilder &AddNull(const char *name);

        Context *Build();
    };


    //
    // Options
    //

    class ROX_API ImpressionHandlerInterface {
    public:
        virtual void HandleImpression(ReportingValue *value,
                                      Experiment *experiment,
                                      Context *context) = 0;

        virtual ~ImpressionHandlerInterface() = default;
    };

    class ROX_API ConfigurationFetchedHandler {
    public:
        virtual void ConfigurationFetched(ConfigurationFetchedArgs *args) = 0;

        virtual ~ConfigurationFetchedHandler() = default;
    };

    class ROX_API DynamicPropertiesRuleInterface {
    public:
        virtual DynamicValue *Invoke(const char *propName, Context *context) = 0;

        virtual ~DynamicPropertiesRuleInterface() = default;
    };

    class ROX_API OptionsBuilder {
    public:

        OptionsBuilder();

        virtual ~OptionsBuilder() = default;

        OptionsBuilder &SetDevModeKey(const char *devModeKey);

        OptionsBuilder &SetVersion(const char *version);

        OptionsBuilder &SetFetchInterval(int intervalInSeconds);

        OptionsBuilder &SetRoxyUrl(const char *roxy_url);

        OptionsBuilder &SetImpressionHandler(ImpressionHandlerInterface *handler);

        OptionsBuilder &SetConfigurationFetchedHandler(ConfigurationFetchedHandler *handler);

        OptionsBuilder &SetDynamicPropertiesRule(DynamicPropertiesRuleInterface *rule);

        Options *Build();

    protected:
        RoxOptions *_options;
    };

    //
    // Setup/Shutdown
    //

    ROX_API void Setup(const char *api_key, Options *options);

    ROX_API void Shutdown();

    //
    // Flags
    //

    class ROX_API Variant {
        friend void Shutdown();

    protected:
        explicit Variant(RoxVariant *variant) : _variant(variant) {}

        RoxVariant *_variant;

        static RoxList *_allVariants;

    public:

        virtual ~Variant() = default;

        char *GetValue(Context *context = nullptr);

        char *GetValueOrNull(Context *context = nullptr);

        static Variant *Create(const char *name, const char *defaultValue);

        static Variant *Create(const char *name, const char *defaultValue, const std::vector<std::string> &options);
    };

    class ROX_API Flag : public Variant {
    protected:
        explicit Flag(RoxVariant *variant) : Variant(variant) {}

    public:

        ~Flag() override = default;

        bool IsEnabled(Context *context = nullptr);

        bool *IsEnabledOrNull(Context *context = nullptr);

        static Flag *Create(const char *name, bool defaultValue = false);
    };

    //
    // Custom Properties
    //

    class ROX_API CustomPropertyGeneratorInterface {
    public:
        /**
         * @param context May be <code>NULL</code>.
         * @return May be <code>NULL</code>.
         */
        virtual DynamicValue *operator()(Context *context) = 0;

        virtual ~CustomPropertyGeneratorInterface() = default;
    };

    template<typename T>
    ROX_API void SetCustomProperty(const char *name, T value);

    template<typename T>
    ROX_API void SetCustomComputedProperty(const char *name, CustomPropertyGeneratorInterface *generator);

    ROX_API void SetCustomSemverProperty(const char *name, const char *value);

    ROX_API void SetCustomComputedSemverProperty(const char *name, CustomPropertyGeneratorInterface *generator);

    //
    // Misc
    //

    ROX_API void SetContext(Context *context);

    ROX_API void Fetch();

    //
    // Dynamic API
    //

    class ROX_API DynamicApi {
    private:
        RoxDynamicApi *_handle;

        explicit DynamicApi(RoxDynamicApi *handle) : _handle(handle) {}

    public:
        virtual ~DynamicApi();

        static DynamicApi *Create();
    };
}
