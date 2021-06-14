#pragma once

extern "C" {
#include <rox/properties.h>
}

#include <roxx/values.h>
#include <roxx/context.h>

namespace Rox {
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
}