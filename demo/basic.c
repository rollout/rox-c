#include <rollout.h>
#include <stdio.h>

#define DEFAULT_API_KEY "5b3356d00d81206da3055bc0"
#define DEFAULT_DEV_MODE_KEY "01fcd0d21eeaed9923dff6d8"

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
        if (c == 0x0A) { // Enter key pressed
            c = 'Y';
        }
        getchar(); // read dummy character to clear input buffer, which inserts after character input
        printf("\n");
    }

    rox_shutdown();
    return 0;
}