#pragma once

extern "C" {
#include <rox/client.h>
}

#include <roxx/dynamic.h>
#include <roxx/logging.h>
#include <roxx/context.h>
#include <roxx/values.h>
#include <roxx/impression.h>
#include <roxx/configuration.h>
#include <roxx/options.h>
#include <roxx/flags.h>
#include <roxx/properties.h>
#include <roxx/setup.h>
#include <roxx/overrides.h>

namespace Rox {

    namespace Client {

        typedef enum RoxFreeze Freeze;

        class ROX_API OptionsBuilder : public Rox::OptionsBuilder {
        public:

            explicit OptionsBuilder() : Rox::OptionsBuilder() {}

            virtual ~OptionsBuilder() = default;

            OptionsBuilder &SetDefaultFreeze(::Rox::Client::Freeze freeze);
        };

        /**
         * @param ns Flag namespace.
         */
        ROX_API void Unfreeze(const char *ns = nullptr);

        class ROX_API String : public Rox::String {
        protected:
            explicit String(RoxStringBase *variant) : Rox::String(variant) {}

        public:

            ~String() override = default;

            void Freeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            void Unfreeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            static String *Create(const char *name, const char *defaultValue,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static String *Create(const char *name, const char *defaultValue, const std::vector<std::string> &options,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Flag : public Rox::Flag {
        protected:
            explicit Flag(RoxStringBase *variant) : Rox::Flag(variant) {}

        public:

            ~Flag() override = default;

            void Freeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            void Unfreeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            static Flag *Create(const char *name, bool defaultValue = false,
                                ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Int : public Rox::Int {
        protected:
            explicit Int(RoxStringBase *variant) : Rox::Int(variant) {}

        public:

            ~Int() override = default;

            void Freeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            void Unfreeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            static Int *Create(const char *name, int defaultValue,
                               ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static Int *Create(const char *name, int defaultValue, const std::vector<int> &options,
                               ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Double : public Rox::Double {
        protected:
            explicit Double(RoxStringBase *variant) : Rox::Double(variant) {}

        public:

            ~Double() override = default;

            void Freeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            void Unfreeze(::Rox::Client::Freeze freeze = RoxFreezeUntilLaunch);

            static Double *Create(const char *name, double defaultValue,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static Double *Create(const char *name, double defaultValue, const std::vector<double> &options,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };
    }
}
