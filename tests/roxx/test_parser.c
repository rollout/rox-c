#include <check.h>

#include "roxx/parser.h"

START_TEST (test_simple_tokenization) {
    // TODO: implement!
}

END_TEST

Suite *parser_suite(void) {
    Suite *suite = suite_create("parser_suite");
    TCase *tcase = tcase_create("test_simple_tokenization");
    tcase_add_test(tcase, test_simple_tokenization);
    suite_add_tcase(suite, tcase);
    return suite;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *suite = parser_suite();
    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}
