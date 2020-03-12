#include <rollout.h>
#include <stdio.h>
#include <assert.h>

#define DEFAULT_API_KEY "5e6a3533d3319d76d1ca33fd"
#define DEFAULT_DEV_MODE_KEY "297c23e7fcb68e54c513dcca"
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
    fclose(file);
    return 0;
}