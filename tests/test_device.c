#include "device.h"
#include "roxtests.h"

START_TEST (test_device_id) {
    const char *device_id = rox_globally_unique_device_id();
    ck_assert_ptr_nonnull(device_id);
    ck_assert(strlen(device_id) > 0);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_device_id)
)
