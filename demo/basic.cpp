#include <rollout.hpp>
#include <cstdio>

#define DEFAULT_API_KEY "5b3356d00d81206da3055bc0"
#define DEFAULT_DEV_MODE_KEY "01fcd0d21eeaed9923dff6d8"

int main(int argc, char **argv) {

    Rox::Logging::SetLogLevel(Rox::LogLevel::RoxLogLevelDebug);

    Rox::Flag *demoFlag = Rox::Flag::Create("demo.demoFlag", false);
    Rox::Options *options = Rox::OptionsBuilder()
            .SetDevModeKey(DEFAULT_DEV_MODE_KEY)
            .Build();

    Rox::Setup(DEFAULT_API_KEY, options);

    char c = 'Y';
    while (c != 'n' && c != 'N') {
        printf("Demo flag is %s\n", demoFlag->IsEnabled() ? "ON" : "OFF");
        printf("Continue? (Y/n):");
        c = fgetc(stdin);
        if (c == 0x0A) { // Enter key pressed
            c = 'Y';
        }
        getchar(); // read dummy character to clear input buffer, which inserts after character input
        printf("\n");
    }

    Rox::Shutdown();
    return 0;
}