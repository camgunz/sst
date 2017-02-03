#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "value.h"

#include "data.h"

void test_add(void **state) {
    String s1;
    String s2;
    SSlice ss1;
    SSlice ss2;
    Value v1;
    Value v2;
    Value v3;
    char *result;
    Status status;
    DecimalContext ctx;

    (void)state;

    status_init(&status);

    decimal_context_set_max(&ctx);

    assert_true(string_init(&s1, NUMBER1, &status));
    assert_true(string_init(&s2, NUMBER2, &status));

    assert_true(string_slice(&s1, 0, s1.len, &ss1, &status));
    assert_true(string_slice(&s2, 0, s2.len, &ss2, &status));

    assert_true(value_init_number_from_sslice(&v1, &ss1, &ctx, &status));
    assert_true(value_init_number_from_sslice(&v2, &ss2, &ctx, &status));
    assert_true(value_init_number(&v3, "0", &ctx, &status));

    assert_true(value_add(&v3, &v1, &v2, &ctx, &status));

    assert_true(value_to_cstr(&result, &v3, &status));

    assert_string_equal(result, NUMBER3);

    free(result);

    value_clear(&v1);
    value_clear(&v2);
    value_clear(&v3);

    string_free(&s1);
    string_free(&s2);
}

/* vi: set et ts=4 sw=4: */
