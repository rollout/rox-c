#include <rox/server.h>
#include <stdio.h>

#define DEFAULT_API_KEY "5e6a3533d3319d76d1ca33fd"
#define DEFAULT_DEV_MODE_KEY "297c23e7fcb68e54c513dcca"

int main(int argc, char **argv) {

    RoxVariant *demo_flag = rox_add_flag("demo.demoFlag", false);
    RoxOptions *options = rox_options_create();
    rox_options_set_dev_mode_key(options, DEFAULT_DEV_MODE_KEY);
    rox_setup(DEFAULT_API_KEY, options);

    char c = 'Y';
    while (c != 'n' && c != 'N') {
        printf("Demo flag is %s\n", rox_flag_is_enabled(demo_flag) ? "ON" : "OFF");
        printf("Continue? (Y/n):");
        c = fgetc(stdin);
        if ((c == '\n' || c == EOF)) { // Enter key pressed
            c = 'Y';
        } else {
            getchar(); // read dummy character to clear input buffer, which inserts after character input
        }
        printf("\n");
    }

    rox_shutdown();
    return 0;
}