#include <stdbool.h>
#include "os.h"
#include "device.h"
#include "util.h"
#include "core/logging.h"

#ifdef ROX_WINDOWS

#include <windows.h>

#else
#include <unistd.h>
#endif

#ifdef ROX_APPLE
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

#define ROX_MACHINE_ID_BUFFER_LENGTH 128

static const char *ROX_DEVICE_ID = NULL;

static char *get_device_id() {
#ifdef ROX_FREE_BSD
    char buffer[ROX_MACHINE_ID_BUFFER_LENGTH];
    if (rox_file_read_b("/etc/hostid", (unsigned char*)buffer, ROX_MACHINE_ID_BUFFER_LENGTH) == -1) {
        ROX_ERROR("Failed to read /etc/hostid");
        return NULL;
    }
    return mem_copy_str(buffer);
#elif defined(ROX_LINUX)
    char buffer[ROX_MACHINE_ID_BUFFER_LENGTH];
    if (rox_file_read_b("/var/lib/dbus/machine-id", (unsigned char*)buffer, ROX_MACHINE_ID_BUFFER_LENGTH) == -1) {
        ROX_ERROR("Failed to read /var/lib/dbus/machine-id");
        return NULL;
    }
    return mem_copy_str(buffer);
#elif defined(ROX_APPLE)
    // TODO: implement!
#elif defined(ROX_WINDOWS)
    HKEY key = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0,
                     KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
        char buffer[ROX_MACHINE_ID_BUFFER_LENGTH + 1];
        DWORD size = sizeof(buffer);
        bool found = (RegQueryValueEx(key, "MachineGuid", NULL, NULL, (LPBYTE) buffer, &size) == ERROR_SUCCESS);
        RegCloseKey(key);
        if (found) {
            return mem_copy_str(buffer);
        }
    }
#endif
    return NULL;
}

#undef ROX_MACHINE_ID_BUFFER_LENGTH

const char *ROX_INTERNAL rox_globally_unique_device_id() {
    if (!ROX_DEVICE_ID) {
        ROX_DEVICE_ID = get_device_id();
    }
    return ROX_DEVICE_ID;
}
