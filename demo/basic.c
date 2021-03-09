#include <rox/server.h>
#include "demo.h"

int main(int argc, char **argv) {

    RoxStringBase *demo_flag = rox_add_flag("demo.demoFlag", false);
    RoxOptions *options = rox_options_create();
    rox_options_set_dev_mode_key(options, DEFAULT_DEV_MODE_KEY);
    rox_setup(DEFAULT_API_KEY, options);

    char c = 'y';
    while (c != 'n') {
        printf("Demo flag is %s\n", rox_is_enabled(demo_flag) ? "ON" : "OFF");
        c = prompt("Continue? (Y/n):");
    }

    rox_shutdown();
    return 0;
}