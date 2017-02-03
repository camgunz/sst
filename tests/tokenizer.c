#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"

#include "data.h"

static inline bool tokenizer_expect(Tokenizer *tokenizer, TokenType type,
                                                          Status *status) {
    return tokenizer_load_next(tokenizer, status) &&
           tokenizer->token.type == type;
}

static inline bool tokenizer_expect_keyword(Tokenizer *tokenizer,
                                            Keyword kw,
                                            Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_KEYWORD, status) &&
           tokenizer->token.as.keyword == kw;
}

static inline bool tokenizer_expect_whitespace(Tokenizer *tokenizer,
                                               Whitespace ws,
                                               Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_WHITESPACE, status) &&
           tokenizer->token.as.whitespace == ws;
}

static inline bool tokenizer_expect_space(Tokenizer *tokenizer,
                                          Status *status) {
    return tokenizer_expect_whitespace(tokenizer, WHITESPACE_SPACE, status);
}

static inline bool tokenizer_expect_string(Tokenizer *tokenizer,
                                           const char *string,
                                           Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_STRING, status) &&
           sslice_equals_cstr(&tokenizer->token.as.string, string);
}

static inline bool tokenizer_expect_text(Tokenizer *tokenizer,
                                         const char *text,
                                         Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_TEXT, status) &&
           sslice_equals_cstr(&tokenizer->token.as.text, text);
}

static inline bool tokenizer_expect_identifier(Tokenizer *tokenizer,
                                               const char *identifier,
                                               Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_IDENTIFIER, status) &&
           sslice_equals_cstr(&tokenizer->token.as.identifier, identifier);
}

static inline bool tokenizer_expect_symbol(Tokenizer *tokenizer,
                                           Symbol symbol,
                                           Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_SYMBOL, status) &&
           tokenizer->token.as.symbol == symbol;
}

static inline bool tokenizer_expect_number(Tokenizer *tokenizer,
                                           const char *number,
                                           Status *status) {
    return tokenizer_expect(tokenizer, TOKEN_NUMBER, status) &&
           sslice_equals_cstr(&tokenizer->token.as.number, number);
}

void test_tokenizer(void **state) {
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
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "/srv/http/templates/header.txt",
        &status
    
    ));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IF, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_identifier(
        &tokenizer, "person.age", &status
    ));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_symbol(
        &tokenizer, SYMBOL_GREATER_THAN_OR_EQUAL, &status
    ));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "18", &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n...\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDIF, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_FOR, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "person", &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IN, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
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
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_ASTERISK, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "tax_rate", &status));
    assert_true(tokenizer_expect_text(&tokenizer, " this year!\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDFOR, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n\n", &status));
    assert_true(tokenizer_expect_text(
        &tokenizer,
        "\n    This is how you would use raw }} and {{ markers.\n",
        &status
    ));
    assert_true(tokenizer_expect_text(&tokenizer, "\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IF, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_identifier(
        &tokenizer, "upper", &status
    ));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_OPAREN, &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "message", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_CPAREN, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_EQUAL, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "OR PUT THEM IN STRINGS {{ }} }} {{",
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
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_FOR, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "fib", &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_IN, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_OBRACKET, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "1", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_COMMA, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "1", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_COMMA, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "2", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_COMMA, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "3", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_COMMA, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "5", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_COMMA, &status));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_number(&tokenizer, "8", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_CBRACKET, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n    Double ", &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "fib", &status));
    assert_true(tokenizer_expect_text(&tokenizer, ": ", &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "double", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_OPAREN, &status));
    assert_true(tokenizer_expect_identifier(&tokenizer, "fib", &status));
    assert_true(tokenizer_expect_symbol(&tokenizer, SYMBOL_CPAREN, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n", &status));
    assert_true(tokenizer_expect_keyword(&tokenizer, KEYWORD_ENDFOR, &status));
    assert_true(tokenizer_expect_text(&tokenizer, "\n", &status));
    assert_true(tokenizer_expect_keyword(
        &tokenizer, KEYWORD_INCLUDE, &status
    ));
    assert_true(tokenizer_expect_space(&tokenizer, &status));
    assert_true(tokenizer_expect_string(
        &tokenizer,
        "/srv/http/templates/footer.txt",
        &status
    ));
    assert_true(tokenizer_expect_text(
        &tokenizer, "\nLast little bit down here\n", &status
    ));
}

/* vi: set et ts=4 sw=4: */
