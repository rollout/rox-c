#pragma once

#include <ctime>

extern "C" {
#include <rox/context.h>
#include <rox/collections.h>
}

namespace Rox {

    typedef struct RoxContext Context;

    class ROX_API ContextBuilder {
    private:
        RoxMap *_map;

    public:
        ContextBuilder();

        ContextBuilder &AddBoolValue(const char *name, bool value);

        ContextBuilder &AddIntValue(const char *name, int value);

        ContextBuilder &AddDoubleValue(const char *name, double value);

        ContextBuilder &AddStringValue(const char *name, const char *value);

        ContextBuilder &AddDateTimeValue(const char *name, const struct tm *value);

        ContextBuilder &AddUndefined(const char *name);

        ContextBuilder &AddNull(const char *name);

        Context *Build();
    };
}