#include <rox/server.h>
#include <assert.h>
#include "demo.h"

#define LOG_FILE_PATH "./logging-output.log"

static void _file_logging_handler(void *target, RoxLogMessage *message) {
    assert(target);
    assert(message);
    FILE *file = (FILE *) target;
    fprintf(file, "(%s) %s\n", message->level_name, message->message);
    fflush(file);
}

int main(int argc, char **argv) {
    FILE *file = fopen(LOG_FILE_PATH, "w");
    if (!file) {
        fprintf(stderr, "Cannot open %s\n", LOG_FILE_PATH);
        return -1;
    }

    RoxLoggingConfig cfg = ROX_LOGGING_CONFIG_INITIALIZER(RoxLogLevelDebug);
    cfg.target = file;
    cfg.handler = &_file_logging_handler;
    rox_logging_init(&cfg);

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
    fclose(file);
    return 0;
}