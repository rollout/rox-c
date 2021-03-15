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
        const char *const _message;
        StateCode _code;
    public:

        explicit SetupException(const char *const message, StateCode code)
                : exception(), _code(code), _message(message) {};

        StateCode GetCode() { return _code; }

        const char *what() const noexcept override {
            return _message;
        }
    };
}
