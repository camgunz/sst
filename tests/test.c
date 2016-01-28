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
#include "splitter.h"
#include "value.h"

#define NUMBER1 "82349023489234902342323419041892349034189341.143"
#define NUMBER2 "9378334023462303455668934502028340345.2890383143"

#define TEMPLATE \
"{{ include '/srv/http/templates/header.tmpl' }}\n" \
"<html>\n"                                          \
"<head>\n"                                          \
"<title>{{ page.title }}</title>\n"


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

void test_splitter(void) {
    Splitter splitter;

    splitter_init(&splitter, TEMPLATE);

    while (splitter_load_next(&splitter)) {
        g_printf("Section - is_code: %u\n", splitter.section_is_code);
        if (splitter.section_is_code) {
            char *code = strndup(splitter.section.data, splitter.section.len);

            g_printf("Code: [%s]\n", code);
        }
    }
}

void test_string(void) {
    String s;
    char *cs = strdup(NUMBER1);
    gunichar uc;

    string_assign(&s, cs);
    g_printf("Equals: %u\n", string_equals(&s, cs));
    g_printf("Starts with 8234: %u\n", string_starts_with(&s, "8234"));
    g_printf("String length: %u\n", s.len);

    string_first_char(&s, &uc);
    g_printf("First char: %c\n", uc);

    uc = 0;

    string_pop_char(&s, &uc);
    g_printf("Popped first char: %c\n", uc);
    g_printf("String length: %u\n", s.len);

    g_printf("Second char is '2': %u\n", string_first_char_equals(&s, '2'));

    string_pop_char_if_equals(&s, '2');
    g_printf("Second char: %u\n", string_first_char_equals(&s, '2'));
    g_printf("String length: %u\n", s.len);

    string_pop_char_if_digit(&s, &uc);
    g_printf("Third char: %c\n", uc);
    g_printf("String length: %u\n", s.len);

    string_pop_char_if_alnum(&s, &uc);
    g_printf("Fourth char: %c\n", uc);
    g_printf("String length: %u\n", s.len);

    g_printf("Find 0: %s\n", string_find(&s, '0'));
}

int main(void) {
    test_mpfr();
    test_add();
    test_splitter();
    test_string();

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */

