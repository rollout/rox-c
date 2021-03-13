#pragma once

extern "C" {
#include <rox/setup.h>
#include <rox/errors.h>
}

#include <roxx/context.h>
#include <roxx/options.h>

namespace Rox {

    typedef enum RoxStateCode StateCode;

    ROX_API StateCode Setup(const char *api_key, Options *options = nullptr);

    ROX_API void SetContext(Context *context);

    ROX_API void Fetch();

    ROX_API void Shutdown();

    class ROX_API SetupException : public std::exception {
    private:
        StateCode _code;
    public:

        explicit SetupException(const char *const message, StateCode code) : exception(message), _code(code) {};

        StateCode GetCode() { return _code; }
    };
}
