#pragma once

extern "C" {
#include <rox/configuration.h>
}

#include <roxx/values.h>

namespace Rox {

    typedef struct RoxConfigurationFetchedArgs ConfigurationFetchedArgs;

    class ROX_API ConfigurationFetchedHandlerInterface {
    public:
        virtual void ConfigurationFetched(ConfigurationFetchedArgs *args) = 0;

        virtual ~ConfigurationFetchedHandlerInterface() = default;
    };

    class ROX_API DynamicPropertiesRuleInterface {
    public:
        virtual DynamicValue *Invoke(const char *propName, Context *context) = 0;

        virtual ~DynamicPropertiesRuleInterface() = default;
    };
}