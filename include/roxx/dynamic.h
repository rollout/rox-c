#pragma once

extern "C" {
#include <rox/macros.h>
}

#include <vector>
#include <string>
#include <roxx/context.h>

namespace Rox {
    class ROX_API DynamicApi {
    private:
        RoxDynamicApi *_handle;

        explicit DynamicApi(RoxDynamicApi *handle) : _handle(handle) {}

    public:
        virtual ~DynamicApi();

        static DynamicApi *Create();

        bool IsEnabled(const char *name,
                       bool default_value = false,
                       Context *context = nullptr);

        int GetInt(const char *name,
                   int default_value,
                   Context *context = nullptr);

        int GetInt(const char *name,
                   int default_value,
                   const std::vector<int> &options,
                   Context *context);

        double GetDouble(const char *name,
                         double default_value,
                         Context *context = nullptr);

        double GetDouble(const char *name,
                         double default_value,
                         const std::vector<double> &options,
                         Context *context);

        char *GetString(const char *name,
                        char *default_value = nullptr,
                        Context *context = nullptr);

        char *GetString(const char *name,
                        char *default_value,
                        const std::vector<std::string> &options,
                        Context *context);
    };
}