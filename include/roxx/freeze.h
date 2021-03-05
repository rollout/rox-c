#pragma once

extern "C" {
#include <rox/freeze.h>
}

#include <roxx/flags.h>

namespace Rox {

    typedef enum RoxFreeze Freeze;

    static Flag *CreateFlagWithFreeze(
            const char *name,
            Freeze freeze);

    static Flag *CreateFlagWithFreeze(
            const char *name,
            bool defaultValue,
            Freeze freeze);

    static String *CreateStringWithFreeze(
            const char *name,
            const char *defaultValue,
            Freeze freeze);

    static String *CreateStringWithFreeze(
            const char *name,
            const char *defaultValue,
            const std::vector<std::string> &options,
            Freeze freeze);

    static Int *CreateIntWithFreeze(
            const char *name,
            int defaultValue,
            Freeze freeze);

    static Int *CreateIntWithFreeze(
            const char *name,
            int defaultValue,
            const std::vector<int> &options,
            Freeze freeze);

    static Double *CreateDoubleWithFreeze(
            const char *name,
            double defaultValue,
            Freeze freeze);

    static Double *CreateDoubleWithFreeze(
            const char *name,
            double defaultValue,
            const std::vector<double> &options,
            Freeze freeze);

    static void FreezeFlag(BaseFlag *flag, Freeze freeze);

    static void UnfreezeFlag(BaseFlag *flag, Freeze freeze);

    /**
     * @param ns Flag namespace. Not <code>NULL</code>.
     */
    static void Unfreeze(const char *ns);

    static void Unfreeze();
}
