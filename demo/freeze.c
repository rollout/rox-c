#include <rox/client.h>
#include <stdio.h>
#include "demo.h"

int main(int argc, char **argv) {

    RoxStringBase *demo_flag = rox_add_flag("demo.demoFlag", false);

    RoxStringBase *demo_int = rox_add_int_with_options("demo.demoInt", 1, ROX_INT_LIST(1, 2, 3));

    RoxStringBase *demo_double = rox_add_double_with_options("demo.demoDouble", 1.1, ROX_DBL_LIST(1.1, 2.2, 3.3));

    RoxStringBase *demo_str = rox_add_string_with_freeze_and_options(
            "demo.demoStr", "red", ROX_LIST_COPY_STR("red", "green", "blue"),
            RoxFreezeNone);

    RoxOptions *options = rox_options_create();
    rox_options_set_default_freeze(options, RoxFreezeUntilLaunch);
    rox_options_set_dev_mode_key(options, DEFAULT_DEV_MODE_KEY);
    rox_setup(DEFAULT_API_KEY, options);

    bool loop = true;
    while (loop) {
        printf("Current flag values:\n");
        printf("Flag is %s\n", rox_is_enabled(demo_flag) ? "ON" : "OFF");
        printf("String value is \"%s\"\n", rox_get_string(demo_str));
        printf("Int value is %d\n", rox_get_int(demo_int));
        printf("Double value is %f\n", rox_get_double(demo_double));
        printf("\n");
        switch (prompt("1) Unfreeze double\n2) Unfreeze bool\n3) Unfreeze int\n4) Unfreeze all\n5) Print\n6) Exit")) {
            case '1':
                rox_unfreeze_flag(demo_double, RoxFreezeUntilLaunch);
                break;
            case '2':
                rox_unfreeze_flag(demo_flag, RoxFreezeUntilLaunch);
                break;
            case '3':
                rox_unfreeze_flag(demo_int, RoxFreezeUntilLaunch);
                break;
            case '4':
                rox_unfreeze();
                continue;
            case '5':
                continue;
            case '6':
                loop = false;
                break;
        }
    }

    rox_shutdown();
    return 0;
}