#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "value.h"

#define NUMBER1 "82349023489234902342323419041892349034189341.796"
#define NUMBER2 "13982309378334023462303455668934502028340345.289"
#define NUMBER3 "96331332867568925804626874710826851062529687.085"
#define NUMBER4      "023489234902342323419041892349034189341.796"
#define NUMBER5       "23489234902342323419041892349034189341.796"
#define NUMBER6       "23489234902342323419041892349034189341.79"

#define REAL_TEMPLATE \
"{{ include '/srv/http/templates/header.txt' }}\n"                         \
"\n"                                                                       \
"{{ if person.age >= 18 }}\n"                                              \
"...\n"                                                                    \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ for person in people }}\n"                                             \
"    {{ person.name }} owes {{ person.income * tax_rate }} this year!\n"   \
"{{ endfor }}\n"                                                           \
"\n"                                                                       \
"{{ raw }}\n"                                                              \
"    This is how you would use raw }} and {{ markers.\n"                   \
"{{ endraw }}\n"                                                           \
"{{ if message == 'Or you can put them in strings {{ }} }} {{' }}\n"       \
"You guessed the magic message!\n"                                         \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ include '/srv/http/templates/footer.txt' }}\n"                         \
"Last little bit down here\n"

void die(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

static void test_add(void **state) {
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

    decimal_context_init(&ctx, DECIMAL_MAX_PRECISION);
    decimal_context_set_max(&ctx);

    assert_true(string_init(&s1, NUMBER1, &status));
    assert_true(string_init(&s2, NUMBER2, &status));

    assert_true(string_slice(&s1, 0, s1.len, &ss1, &status));
    assert_true(string_slice(&s2, 0, s2.len, &ss2, &status));

    assert_true(value_init_number_from_sslice(&v1, &ss1, &ctx, &status));
    assert_true(value_init_number_from_sslice(&v2, &ss2, &ctx, &status));

    assert_true(value_add(&v3, &v1, &v2, &ctx, &status));

    assert_true(value_to_cstr(&result, &v3, &status));

    assert_string_equal(result, NUMBER3);

    free(result);

    assert_true(value_clear(&v1, &status));
    assert_true(value_clear(&v2, &status));
    assert_true(value_clear(&v3, &status));

    string_free(&s1);
    string_free(&s2);
}

static void test_tokenizer(void **state) {
    String s;
    SSlice ss;
    Tokenizer tokenizer;
    Status status;

    (void)state;

    status_init(&status);

    assert_true(string_init(&s, REAL_TEMPLATE, &status));
    assert_true(string_slice(&s, 0, s.len, &ss, &status));

    tokenizer_init(&tokenizer, &ss);

    while (tokenizer_load_next(&tokenizer, &status)) {
        char *token_data = NULL;

        switch (tokenizer.token.type) {
            case TOKEN_TEXT:
                token_data = sslice_to_cstr(&tokenizer.token.as.text);
                printf("<TEXT: %s>\n", token_data);
                break;
            case TOKEN_NUMBER:
                token_data = sslice_to_cstr(&tokenizer.token.as.number);
                printf("<NUMBER: %s>\n", token_data);
                break;
            case TOKEN_STRING:
                token_data = sslice_to_cstr(&tokenizer.token.as.string);
                printf("<STRING: %s>\n", token_data);
                break;
            case TOKEN_SYMBOL:
                printf("<SYMBOL: %s>\n",
                    SymbolValues[tokenizer.token.as.symbol]
                );
                break;
            case TOKEN_KEYWORD:
                printf("<KEYWORD: %s>\n",
                    KeywordValues[tokenizer.token.as.keyword]
                );
                break;
            case TOKEN_IDENTIFIER:
                token_data = sslice_to_cstr(&tokenizer.token.as.identifier);
                printf("<IDENTIFIER: %s>\n", token_data);
                break;
            case TOKEN_WHITESPACE:
                switch (tokenizer.token.as.whitespace) {
                    case WHITESPACE_SPACE:
                        puts("<WHITESPACE: (space)>");
                        break;
                    case WHITESPACE_TAB:
                        puts("<WHITESPACE: (tab)>");
                        break;
                    case WHITESPACE_NEWLINE:
                        puts("<WHITESPACE: (tab)>");
                        break;
                    default:
                        die("Invalid whitespace type %d\n",
                            tokenizer.token.as.whitespace
                        );
                        break;
                }
                token_data = sslice_to_cstr(&tokenizer.token.as.text);
                break;
            default:
                die("Invalid token type %d\n", tokenizer.token.type);
                break;
        }

        if (token_data) {
            free(token_data);
        }
    }
}

static void test_lexer(void **state) {
    (void)state;
}

static void test_parser(void **state) {
    (void)state;
}

int main(void) {
    int failed_test_count = 0;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_add),
        cmocka_unit_test(test_tokenizer),
        cmocka_unit_test(test_lexer),
        cmocka_unit_test(test_parser),
    };

    failed_test_count = cmocka_run_group_tests(tests, NULL, NULL);

    if (failed_test_count > 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */
