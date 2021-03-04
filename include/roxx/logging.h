#pragma once

extern "C" {
#include <rox/macros.h>
#include <rox/logging.h>
}

namespace Rox {

    typedef struct RoxLogMessage LogMessage;

    typedef enum RoxLogLevel LogLevel;

    class ROX_API LogMessageHandlerInterface {
    public:
        virtual void HandleLogMessage(LogMessage *message) = 0;

        virtual ~LogMessageHandlerInterface() = default;
    };

    class ROX_API Logging {
    private:
        RoxLoggingConfig _config;

        Logging() : _config(ROX_LOGGING_CONFIG_INITIALIZER(RoxLogLevelError)) {
        }

        // Stop the compiler generating methods of copy the object
        Logging(const Logging &copy) = default;

        Logging &operator=(const Logging &copy) = default;

        static Logging &GetInstance() {
            // The only instance
            // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
            static Logging instance;
            return instance;
        }

    public:
        static void SetLogLevel(LogLevel logLevel);

        static void SetLogMessageHandler(LogMessageHandlerInterface *handler);
    };
}