#include <check.h>

#include "roxtests.h"
#include "util.h"

START_TEST (test_substring_start_offset_out_of_bounds) {
    char *str = str_substring("test", 4, 1);
    ck_assert_ptr_null(str);
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(test_substring_start_offset_out_of_bounds)
);