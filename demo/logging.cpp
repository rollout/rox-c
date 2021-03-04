#include <roxx/server.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#define DEFAULT_API_KEY "5e6a3533d3319d76d1ca33fd"
#define DEFAULT_DEV_MODE_KEY "297c23e7fcb68e54c513dcca"
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

    char c = 'Y';
    while (c != 'n' && c != 'N') {
        printf("Demo flag is %s\n", demoFlag->IsEnabled() ? "ON" : "OFF");
        printf("Continue? (Y/n):");
        c = fgetc(stdin);
        if ((c == '\n' || c == EOF)) { // Enter key pressed
            c = 'Y';
        } else {
            getchar(); // read dummy character to clear input buffer, which inserts after character input
        }
        printf("\n");
    }

    Rox::Shutdown();
    return 0;
}
