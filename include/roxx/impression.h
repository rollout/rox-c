#pragma once

extern "C" {
#include <rox/impression.h>
}

namespace Rox {

    typedef struct RoxReportingValue ReportingValue;

    class ROX_API ImpressionHandlerInterface {
    public:
        virtual void HandleImpression(ReportingValue *value, Context *context) = 0;

        virtual ~ImpressionHandlerInterface() = default;
    };
}