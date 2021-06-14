#include <rox/client.h>
#include <stdio.h>
#include "demo.h"

int main(int argc, char **argv) {

    RoxStringBase *demo_flag = rox_add_flag("demo.demoFlag", false);
    RoxStringBase *demo_int = rox_add_int_with_options("demo.demoInt", 1, ROX_INT_LIST(1, 2, 3));
    RoxStringBase *demo_double = rox_add_double_with_options("demo.demoDouble", 1.1, ROX_DBL_LIST(1.1, 2.2, 3.3));
    RoxStringBase *demo_str = rox_add_string_with_options("demo.demoStr", "red",
                                                          ROX_LIST_COPY_STR("red", "green", "blue"));

    RoxOptions *options = rox_options_create();
    rox_options_set_dev_mode_key(options, DEFAULT_DEV_MODE_KEY);
    rox_setup(DEFAULT_API_KEY, options);

    RoxFlagOverrides *overrides = rox_get_overrides();
    rox_set_override(overrides, "demo.demoFlag", "true");
    rox_set_override(overrides, "demo.demoInt", "2");
    rox_set_override(overrides, "demo.demoDouble", "3.3");
    rox_set_override(overrides, "demo.demoStr", "blue");

    bool loop = true;
    while (loop) {
        printf("Current flag values:\n");

        printf("Flag is %s%s\n",
               rox_is_enabled(demo_flag) ? "ON" : "OFF",
               rox_has_override(overrides, "demo.demoFlag") ? " (overridden)" : "");

        printf("String value is \"%s\"%s\n",
               rox_get_string(demo_str),
               rox_has_override(overrides, "demo.demoStr") ? " (overridden)" : "");

        printf("Int value is %d%s\n",
               rox_get_int(demo_int),
               rox_has_override(overrides, "demo.demoInt") ? " (overridden)" : "");

        printf("Double value is %f%s\n",
               rox_get_double(demo_double),
               rox_has_override(overrides, "demo.demoDouble") ? " (overridden)" : "");

        printf("\n");
        switch (prompt("1) Print values\n2) Clear overrides\n3) Exit")) {
            case '1':
                continue;
            case '2':
                rox_clear_overrides(overrides);
                continue;
            case '3':
                loop = false;
                break;
        }
    }

    rox_shutdown();
    return 0;
}