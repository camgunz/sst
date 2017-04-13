#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

void test_add(void **state);
void test_tokenizer(void **state);
void test_lexer(void **state);
void test_parser(void **state);

int main(void) {
    int failed_test_count = 0;

    decimal_set_minalloc(8);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_add), /* 5 (7), 1,341 */
        cmocka_unit_test(test_tokenizer), /* 1 (3), 1,978 */
        cmocka_unit_test(test_lexer), /* 2 (4), 2,042 */
        cmocka_unit_test(test_parser), /* 5 (7), 3,834 */
    };

    failed_test_count = cmocka_run_group_tests(tests, NULL, NULL);

    if (failed_test_count > 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */
