
#pragma once

extern "C" {
#include <rox/flags.h>
}

#include <vector>
#include <string>
#include <roxx/context.h>
#include <roxx/setup.h>

namespace Rox {

    class ROX_API BaseFlag {
        friend void Shutdown();

    protected:
        RoxStringBase *_variant;

        static RoxList *_allVariants;

        explicit BaseFlag(RoxStringBase *variant);

    public:
        virtual ~BaseFlag() = default;

        const char *GetName();
    };

    class ROX_API String : protected BaseFlag {
    protected:
        explicit String(RoxStringBase *variant) : BaseFlag(variant) {}

    public:

        ~String() override = default;

        char *GetValue(Context *context = nullptr);

        static String *Create(const char *name, const char *defaultValue);

        static String *Create(const char *name, const char *defaultValue, const std::vector<std::string> &options);
    };

    class ROX_API Flag : protected BaseFlag {
    protected:
        explicit Flag(RoxStringBase *variant) : BaseFlag(variant) {}

    public:

        ~Flag() override = default;

        bool IsEnabled(Context *context = nullptr);

        static Flag *Create(const char *name, bool defaultValue = false);
    };

    class ROX_API Int : protected BaseFlag {
    protected:
        explicit Int(RoxStringBase *variant) : BaseFlag(variant) {}

    public:

        ~Int() override = default;

        int GetValue(Context *context = nullptr);

        static Int *Create(const char *name, int defaultValue);

        static Int *Create(const char *name, int defaultValue, const std::vector<int> &options);
    };

    class ROX_API Double : protected BaseFlag {
    protected:
        explicit Double(RoxStringBase *variant) : BaseFlag(variant) {}

    public:

        ~Double() override = default;

        double GetValue(Context *context = nullptr);

        static Double *Create(const char *name, double defaultValue);

        static Double *Create(const char *name, double defaultValue, const std::vector<double> &options);
    };
}