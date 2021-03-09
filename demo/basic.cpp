#include <roxx/server.h>
#include "demo.hpp"

int main(int argc, char **argv) {

    Rox::Flag *demoFlag = Rox::Flag::Create("demo.demoFlag", false);
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