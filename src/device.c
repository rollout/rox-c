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

static char system_device_id[ROX_MACHINE_ID_BUFFER_LENGTH];
static bool system_device_id_initialized = false;

static char *get_device_id() {
#if defined(ROX_FREE_BSD)
    if (rox_file_read_b("/etc/hostid", (unsigned char*)system_device_id, ROX_MACHINE_ID_BUFFER_LENGTH) == -1) {
        ROX_ERROR("Failed to read /etc/hostid");
        return NULL;
    }
    return system_device_id;
#elif defined(ROX_LINUX)
    if (rox_file_read_b("/var/lib/dbus/machine-id", (unsigned char*)system_device_id, ROX_MACHINE_ID_BUFFER_LENGTH) == -1) {
        ROX_ERROR("Failed to read /var/lib/dbus/machine-id");
        return NULL;
    }
    return system_device_id;
#elif defined(ROX_APPLE)
    io_registry_entry_t registry_entry = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    if (!registry_entry) {
        ROX_ERROR("IORegistryEntryFromPath failed");
        return NULL;
    }
    CFStringRef uuid_ref = (CFStringRef)IORegistryEntryCreateCFProperty(registry_entry, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(registry_entry);
    if (!uuid_ref) {
        ROX_ERROR("IORegistryEntryCreateCFProperty failed");
        return NULL;
    }
    if (!CFStringGetCString(uuid_ref, system_device_id, ROX_MACHINE_ID_BUFFER_LENGTH, kCFStringEncodingASCII)) {
        ROX_ERROR("CFStringGetCString failed");
        CFRelease(uuid_ref);
        return NULL;
    }
    CFRelease(uuid_ref);
    return system_device_id;
#elif defined(ROX_WINDOWS)
    HKEY key = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0,
                     KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
        DWORD size = sizeof(system_device_id);
        bool found = (RegQueryValueEx(key, "MachineGuid", NULL, NULL, (LPBYTE) system_device_id, &size) ==
                      ERROR_SUCCESS);
        RegCloseKey(key);
        if (found) {
            return system_device_id;
        }
    }
#endif
    return NULL;
}

#undef ROX_MACHINE_ID_BUFFER_LENGTH

ROX_INTERNAL const char *rox_globally_unique_device_id() {
    if (system_device_id_initialized) {
        return system_device_id;
    }
    char *device_id = get_device_id();
    if (device_id != NULL) {
        system_device_id_initialized = true;
    }
    return device_id;
}
