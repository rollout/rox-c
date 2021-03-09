#pragma once

extern "C" {
#include <rox/options.h>
}

#include <roxx/configuration.h>
#include <roxx/impression.h>

namespace Rox {

    typedef struct RoxOptions Options;

    class ROX_API OptionsBuilder {
    public:

        OptionsBuilder();

        virtual ~OptionsBuilder() = default;

        OptionsBuilder &SetDevModeKey(const char *devModeKey);

        OptionsBuilder &SetVersion(const char *version);

        OptionsBuilder &SetFetchInterval(int intervalInSeconds);

        OptionsBuilder &SetRoxyUrl(const char *roxy_url);

        OptionsBuilder &SetImpressionHandler(ImpressionHandlerInterface *handler);

        OptionsBuilder &SetConfigurationFetchedHandler(ConfigurationFetchedHandlerInterface *handler);

        OptionsBuilder &SetDynamicPropertiesRule(DynamicPropertiesRuleInterface *rule);

        Options *Build();

    protected:
        RoxOptions *_options;
    };
}