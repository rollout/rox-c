#include <roxx/server.h>
#include <iostream>
#include <fstream>
#include "demo.hpp"

#define LOG_FILE_PATH "./logging-output-cpp.log"

class StreamLoggingHandler : public Rox::LogMessageHandlerInterface {
    std::ostream &_stream;
public:
    explicit StreamLoggingHandler(std::ostream &stream) : _stream(stream) {}

    void HandleLogMessage(Rox::LogMessage *message) override {
        _stream << "(" << message->level_name << ")" << " " << message->message << "\n";
        _stream.flush();
    }
};

int main(int argc, char **argv) {

    std::ofstream stream(LOG_FILE_PATH);
    if (!stream.is_open()) {
        std::cerr << "Cannot open " << LOG_FILE_PATH << "\n";
        return -1;
    }

    StreamLoggingHandler loggingHandler(stream);

    Rox::Logging::SetLogLevel(RoxLogLevelDebug);
    Rox::Logging::SetLogMessageHandler(&loggingHandler);

    Rox::Flag *demoFlag = Rox::Flag::Create("demo.demoFlag");
    Rox::Options *options = Rox::OptionsBuilder()
            .SetDevModeKey(DEFAULT_DEV_MODE_KEY)
            .Build();

    Rox::Setup(DEFAULT_API_KEY, options);

    char c = 'y';
    while (c != 'n') {
        printf("Demo flag is %s\n", demoFlag->IsEnabled() ? "ON" : "OFF");
        c = prompt("Continue? (Y/n):");
    }

    Rox::Shutdown();
    return 0;
}
