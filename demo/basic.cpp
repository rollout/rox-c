#include <roxx/server.h>
#include <cstdio>

#define DEFAULT_API_KEY "5e6a3533d3319d76d1ca33fd"
#define DEFAULT_DEV_MODE_KEY "297c23e7fcb68e54c513dcca"

int main(int argc, char **argv) {

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