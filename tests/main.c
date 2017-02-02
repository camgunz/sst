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

#define TEMPLATE \
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

static inline bool tokenizer_expect(Tokenizer *tokenizer, TokenType type,
                                                          Status *status) {
    if (!tokenizer_load_next(tokenizer, status)) {
        return false;
    }

    return tokenizer->token.type == type;
}

static inline bool tokenizer_expect_keyword(Tokenizer *tokenizer,
                                            Keyword kw,
                                            Status *status) {
    if (!tokenizer_expect(tokenizer, TOKEN_KEYWORD, status)) {
        return false;
    }

    return tokenizer->token.as.keyword == kw;
}

static inline bool tokenizer_expect_whitespace(Tokenizer *tokenizer,
                                               Whitespace ws,
                                               Status *status) {
    if (!tokenizer_expect(tokenizer, TOKEN_WHITESPACE, status)) {
        return false;
    }

    return tokenizer->token.as.whitespace == ws;
}

static inline bool tokenizer_expect_string(Tokenizer *tokenizer,
                                           const char *s,
                                           Status *status) {
    static String buf;
    static bool initialized = false;

    if (!initialized) {
        assert_true(string_init(&buf, "", status));
        initialized = true;
    }
    else {
        string_clear(&buf);
    }

    if (!tokenizer_expect(tokenizer, TOKEN_STRING, status)) {
        return false;
    }

    if (!string_init_from_sslice(&buf, &tokenizer->token.as.string, status)) {
        return false;
    }

    return strcmp(buf.data, s) == 0;
}

static inline bool tokenizer_expect_text(Tokenizer *tokenizer,
                                         const char *s,
                                         Status *status) {
    static String buf;
    static bool initialized = false;

    if (!initialized) {
        assert_true(string_init(&buf, "", status));
        initialized = true;
    }
    else {
        string_clear(&buf);
    }

    if (!tokenizer_expect(tokenizer, TOKEN_TEXT, status)) {
        return false;
    }

    if (!string_init_from_sslice(&buf, &tokenizer->token.as.text, status)) {
        return false;
    }

    return strcmp(buf.data, s) == 0;;
}

static inline bool tokenizer_expect_raw(Tokenizer *tokenizer,
                                        const char *s,
                                        Status *status) {
    static String buf;
    static bool initialized = false;

    if (!initialized) {
        assert_true(string_init(&buf, "", status));
        initialized = true;
    }
    else {
        string_clear(&buf);
    }

    if (!tokenizer_expect(tokenizer, TOKEN_RAW, status)) {
        return false;
    }

    if (!string_init_from_sslice(&buf, &tokenizer->token.as.raw, status)) {
        return false;
    }

    return strcmp(buf.data, s) == 0;;
}

static inline bool tokenizer_expect_identifier(Tokenizer *tokenizer,
                                               const char *s,
                                               Status *status) {
    static String buf;
    static bool initialized = false;

    if (!initialized) {
        assert_true(string_init(&buf, "", status));
        initialized = true;
    }
    else {
        string_clear(&buf);
    }

    if (!tokenizer_expect(tokenizer, TOKEN_IDENTIFIER, status)) {
        return false;
    }

    if (!string_init_from_sslice(&buf, &tokenizer->token.as.identifier,
                                       status)) {
        return false;
    }

    return strcmp(buf.data, s) == 0;
}

static inline bool tokenizer_expect_symbol(Tokenizer *tokenizer,
                                           Symbol symbol,
                                           Status *status) {
    if (!tokenizer_expect(tokenizer, TOKEN_SYMBOL, status)) {
        return false;
    }

    return tokenizer->token.as.symbol == symbol;
}

static inline bool tokenizer_expect_number(Tokenizer *tokenizer,
                                           const char *number,
                                           Status *status) {
    static String buf;
    static bool initialized = false;

    if (!initialized) {
        assert_true(string_init(&buf, "", status));
        initialized = true;
    }
    else {
        string_clear(&buf);
    }

    if (!tokenizer_expect(tokenizer, TOKEN_NUMBER, status)) {
        return false;
    }

    if (!string_init_from_sslice(&buf, &tokenizer->token.as.number, status)) {
        return false;
    }

    return strcmp(buf.data, number) == 0;
}

static void test_tokenizer(void **state) {
    String s;
    SSlice ss;
    Tokenizer tokenizer;
    Status status;
    String buf;

    (void)state;

    status_init(&status);

    assert_true(string_init(&buf, "", &status));

    assert_true(string_init(&s, TEMPLATE, &status));
    assert_true(string_slice(&s, 0, s.len, &ss, &status));

    tokenizer_init(&tokenizer, &ss);
    assert_true(tokenizer_expect_keyword(
        &tokenizer, KEYWORD_INCLUDE, &status
    ));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "/srv/http/templates/header.txt",
        &status
    
    ));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IF, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_identifier(
        &tokenizer, "person.age", &status
    ));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_symbol(
        &tokenizer, SYMBOL_GREATER_THAN_OR_EQUAL, &status
    ));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_number(&tokenizer, "18", &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n...\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDIF, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_FOR, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_identifier(&tokenizer, "person", &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IN, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_identifier(&tokenizer, "people", &status));
    assert_true(tokenizer_expect_text(
        &tokenizer, "\n    ", &status
    ));
    assert_true(tokenizer_expect_identifier(
        &tokenizer, "person.name", &status
    ));
    assert_true(tokenizer_expect_text(&tokenizer, " owes ", &status));
    assert_true(tokenizer_expect_identifier(
        &tokenizer, "person.income", &status
    ));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_ASTERISK, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_identifier(&tokenizer, "tax_rate", &status));
    assert_true(tokenizer_expect_text(&tokenizer, " this year!\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDFOR, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_raw(
        &tokenizer,
        "\n    This is how you would use raw }} and {{ markers.\n",
        &status
    ));
    assert_true(tokenizer_expect_text(&tokenizer, "\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IF, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_identifier(&tokenizer, "message", &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_EQUAL, &status));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "Or you can put them in strings {{ }} }} {{",
        &status
    
    ));
    assert_true(tokenizer_expect_text(
        &tokenizer,
        "\nYou guessed the magic message!\n",
        &status
    
    ));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDIF, &status));
    assert_true(tokenizer_expect_text(
        &tokenizer, "\n\n", &status
    ));
    assert_true(tokenizer_expect_keyword(
        &tokenizer, KEYWORD_INCLUDE, &status
    ));
    assert_true(tokenizer_expect_whitespace(
        &tokenizer, WHITESPACE_SPACE, &status
    ));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "/srv/http/templates/footer.txt",
        &status
    ));
    assert_true(tokenizer_expect_text(
        &tokenizer, "\nLast little bit down here\n", &status
    ));
}

static void test_lexer(void **state) {
    (void)state;
}

static void test_parser(void **state) {
    (void)state;
}

int main(void) {
    int failed_test_count = 0;

    decimal_set_minalloc(8);

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
