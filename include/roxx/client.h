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

namespace Rox {

    namespace Client {

        typedef enum RoxFreeze Freeze;

        class ROX_API IClientFlag {
        public:
            virtual ~IClientFlag() {}

            virtual void Freeze() = 0;

            virtual void Unfreeze() = 0;

            virtual void Freeze(::Rox::Client::Freeze freeze) = 0;

            virtual void Unfreeze(::Rox::Client::Freeze freeze) = 0;

            /**
             * Retrieves the current flag value without freeze, and without invoking impression.
             * The returned value, if not <code>NULL</code>, must be freed by the caller.
             *
             * @return Can be <code>NULL</code>.
             */
            virtual char *PeekCurrentValue() = 0;

            /**
             * Retrieves the original value with no overrides, no freeze, and without invoking impression.
             * The returned value, if not <code>NULL</code>, must be freed by the caller.
             *
             * @return Can be <code>NULL</code>.
             */
            virtual char *PeekOriginalValue() = 0;
        };

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

        class ROX_API String : public Rox::String, public IClientFlag {
        protected:
            explicit String(RoxStringBase *variant) : Rox::String(variant) {}

        public:

            ~String() override = default;

            void Freeze() override;

            void Unfreeze() override;

            void Freeze(::Rox::Client::Freeze freeze) override;

            void Unfreeze(::Rox::Client::Freeze freeze) override;

            virtual char *PeekCurrentValue() override;

            virtual char *PeekOriginalValue() override;

            static String *Create(const char *name, const char *defaultValue,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static String *Create(const char *name, const char *defaultValue, const std::vector<std::string> &options,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Flag : public Rox::Flag, public IClientFlag {
        protected:
            explicit Flag(RoxStringBase *variant) : Rox::Flag(variant) {}

        public:

            ~Flag() override = default;

            void Freeze() override;

            void Unfreeze() override;

            void Freeze(::Rox::Client::Freeze freeze) override;

            void Unfreeze(::Rox::Client::Freeze freeze) override;

            virtual char *PeekCurrentValue() override;

            virtual char *PeekOriginalValue() override;

            static Flag *Create(const char *name, bool defaultValue = false,
                                ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Int : public Rox::Int, public IClientFlag {
        protected:
            explicit Int(RoxStringBase *variant) : Rox::Int(variant) {}

        public:

            ~Int() override = default;

            void Freeze() override;

            void Unfreeze() override;

            void Freeze(::Rox::Client::Freeze freeze) override;

            void Unfreeze(::Rox::Client::Freeze freeze) override;

            virtual char *PeekCurrentValue() override;

            virtual char *PeekOriginalValue() override;

            static Int *Create(const char *name, int defaultValue,
                               ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static Int *Create(const char *name, int defaultValue, const std::vector<int> &options,
                               ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Double : public Rox::Double, public IClientFlag {
        protected:
            explicit Double(RoxStringBase *variant) : Rox::Double(variant) {}

        public:

            ~Double() override = default;

            void Freeze() override;

            void Unfreeze() override;

            void Freeze(::Rox::Client::Freeze freeze) override;

            void Unfreeze(::Rox::Client::Freeze freeze) override;

            virtual char *PeekCurrentValue() override;

            virtual char *PeekOriginalValue() override;

            static Double *Create(const char *name, double defaultValue,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);

            static Double *Create(const char *name, double defaultValue, const std::vector<double> &options,
                                  ::Rox::Client::Freeze freeze = RoxFreezeNone);
        };

        class ROX_API Overrides {
        protected:
            explicit Overrides() {};

        public:

            virtual ~Overrides() = default;

            bool HasOverride(const char *name);

            void SetOverride(const char *name, const char *value);

            const char *GetOverride(const char *name);

            void Clear(const char *name);

            void Clear();

            static Overrides *Get();
        };
    }
}
