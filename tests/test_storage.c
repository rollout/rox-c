#define ROX_CLIENT

#include <stdio.h>
#include "roxtests.h"
#include "rox/storage.h"
#include "storage.h"
#include "util.h"

static const char *LOCATION = "/tmp/rox/storage/tests";
static const char *ENTRY = "test";

static void clean_storage() {
    char *path = mem_str_format("%s/%s.json", LOCATION, ENTRY);
    remove(path);
    free(path);
}

static void write_values(RoxMap *values) {
    RoxStorage *storage = storage_create_with_location(LOCATION);
    RoxStorageEntry *entry = storage_get_entry(storage, ENTRY);
    storage_write_string_key_value_map(entry, values);
    storage_free(storage);
}

static RoxMap *read_values() {
    RoxStorage *storage = storage_create_with_location(LOCATION);
    RoxStorageEntry *entry = storage_get_entry(storage, ENTRY);
    RoxMap *values = storage_read_string_key_value_map(entry);
    storage_free(storage);
    return values;
}

START_TEST (should_read_existing_file) {
    clean_storage();

    ck_assert(mkdirs("/tmp/rox/storage/tests"));
    str_to_file("/tmp/rox/storage/tests/test.json", "{\"a\":\"b\", \"c\":\"d\"}");

    void *a, *c;
    RoxMap *values = read_values();
    ck_assert_ptr_nonnull(values);
    ck_assert_int_eq(2, rox_map_size(values));
    ck_assert(rox_map_get(values, "a", &a));
    ck_assert_str_eq(a, "b");
    ck_assert(rox_map_get(values, "c", &c));
    ck_assert_str_eq(c, "d");
    rox_map_free_with_keys_and_values_cb(values, free, free);

    clean_storage();
}

END_TEST

START_TEST (should_write_then_read_empty_map) {
    clean_storage();

    RoxMap *values = ROX_EMPTY_MAP;
    write_values(values);
    rox_map_free(values);

    RoxMap *read = read_values();
    ck_assert_ptr_nonnull(read);
    ck_assert_int_eq(0, rox_map_size(read));
    rox_map_free(read);

    clean_storage();
}

END_TEST

START_TEST (should_write_then_read) {
    clean_storage();

    RoxMap *values = ROX_MAP(
            "foo", "bar",
            "baz", "tmp"
    );
    write_values(values);
    rox_map_free(values);

    RoxMap *read = read_values();
    ck_assert_ptr_nonnull(read);
    ck_assert_int_eq(2, rox_map_size(read));

    void *foo, *baz;
    ck_assert(rox_map_get(read, "foo", &foo));
    ck_assert_str_eq(foo, "bar");
    ck_assert(rox_map_get(read, "baz", &baz));
    ck_assert_str_eq(baz, "tmp");
    rox_map_free_with_keys_and_values_cb(read, free, free);

    clean_storage();
}

END_TEST

START_TEST (should_write_then_read_large_map) {
    clean_storage();

    const int num = 1000;
    RoxMap *values = ROX_EMPTY_MAP;
    for (int i = 0; i < num; ++i) {
        rox_map_add(values,
                    mem_str_format("key%d", i),
                    mem_str_format("value%d", i));
    }

    write_values(values);
    rox_map_free_with_keys_and_values_cb(values, free, free);

    RoxMap *read = read_values();
    ck_assert_ptr_nonnull(read);
    ck_assert_int_eq(num, rox_map_size(read));

    for (int i = 0; i < num; ++i) {
        char *key = mem_str_format("key%d", i);
        char *expected_value = mem_str_format("value%d", i);
        void *value;
        ck_assert(rox_map_get(read, key, &value));
        ck_assert_str_eq(value, expected_value);
        free(key);
        free(expected_value);
    }

    rox_map_free_with_keys_and_values_cb(read, free, free);

    clean_storage();
}

END_TEST

ROX_TEST_SUITE(
        ROX_TEST_CASE(should_read_existing_file),
        ROX_TEST_CASE(should_write_then_read_empty_map),
        ROX_TEST_CASE(should_write_then_read),
        ROX_TEST_CASE(should_write_then_read_large_map)
)