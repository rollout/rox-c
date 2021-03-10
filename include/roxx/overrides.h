#pragma once

extern "C" {
#include <rox/overrides.h>
}

#include <roxx/flags.h>

namespace Rox {

    class ROX_API Overrides {
    protected:
        Overrides();

    public:

        virtual ~Overrides() = default;

        bool HasOverride(const char *name);

        void SetOverride(const char *name, const char *value);

        const char *GetOverride(const char *name);

        void Clear(const char *name);

        void Clear();

    private:
        RoxFlagOverrides *_handle;
    };

    ROX_API Overrides *GetOverrides();

    /**
     * Retrieves the current flag value without freeze, and without invoking impression.
     * The returned value, if not <code>NULL</code>, must be freed by the caller.
     *
     * @param flag Not <code>NULL</code>.
     * @return Can be <code>NULL</code>.
     */
    ROX_API char *PeekCurrentValue(BaseFlag *flag);

    /**
     * Retrieves the original value with no overrides, no freeze, and without invoking impression.
     * The returned value, if not <code>NULL</code>, must be freed by the caller.
     *
     * @param flag Not <code>NULL</code>.
     * @return Can be <code>NULL</code>.
     */
    ROX_API char *PeekOriginalValue(BaseFlag *flag);
}