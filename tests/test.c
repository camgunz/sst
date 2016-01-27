#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <gmp.h>
#include <mpfr.h>

#include "config.h"
#include "str.h"
#include "utils.h"
#include "value.h"

#define NUMBER1 "82349023489234902342323419041892349034189341.143"
#define NUMBER2 "9378334023462303455668934502028340345.2890383143"

void test_mpfr(void) {
    mpfr_t n;
    mpfr_exp_t ep;

    mpfr_init2(n, DEFAULT_PRECISION);
    mpfr_strtofr(n, NUMBER1, NULL, 0, MPFR_RNDZ);

    g_printf("Number: %s\n", mpfr_get_str(NULL, &ep, 10, 0, n, MPFR_RNDZ));
}

void test_add(void) {
    Value v1;
    Value v2;
    Value v3;
    String s1;
    String s2;
    char *result;

    value_init(&v1);
    value_init(&v2);
    value_init(&v3);

    string_assign(&s1, NUMBER1);
    string_assign(&s2, NUMBER2);

    value_set_number(&v1, &s1);
    value_set_number(&v2, &s2);

    value_add(&v3, &v1, &v2);
    value_as_string(&result, &v3);

    g_printf("Result: %s + %s = %s\n", NUMBER1, NUMBER2, result);
}

int main(void) {
    test_mpfr();
    test_add();

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */

