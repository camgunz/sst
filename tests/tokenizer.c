#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"

#include "data.h"

#define texpect(token_type) \
    if (!tokenizer_load_next(&tokenizer, &status)) { \
        printf("tokenizer_load_next failed: %s\n", status.message); \
        assert_true(false); \
    } \
    assert_int_equal(tokenizer.token.type, token_type)

#define texpect_keyword(kw) \
    texpect(TOKEN_KEYWORD); \
    assert_int_equal(tokenizer.token.as.keyword, kw)

#define texpect_space() \
    texpect(TOKEN_SPACE)

#define texpect_string(str) \
    texpect(TOKEN_STRING); \
    assert_true(sslice_equals_cstr(&tokenizer.token.as.string, str))

#define texpect_text(txt) \
    texpect(TOKEN_TEXT); \
    assert_true(sslice_equals_cstr(&tokenizer.token.as.text, txt))

#define texpect_identifier(ident) \
    texpect(TOKEN_IDENTIFIER); \
    assert_true(sslice_equals_cstr(&tokenizer.token.as.identifier, ident))

#define texpect_symbol(sym) \
    texpect(TOKEN_SYMBOL); \
    assert_int_equal(tokenizer.token.as.symbol, sym)

#define texpect_number(num) \
    texpect(TOKEN_NUMBER); \
    assert_true(sslice_equals_cstr(&tokenizer.token.as.number, num))

#define texpect_code_start() \
    texpect(TOKEN_CODE_START)

#define texpect_code_end() \
    texpect(TOKEN_CODE_END)

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

    texpect_code_start();
    texpect_keyword(KEYWORD_INCLUDE);
    texpect_space();
    texpect_string("/srv/http/templates/header.txt");
    texpect_code_end();
    texpect_text("\n\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_IF);
    texpect_space();
    texpect_identifier("person.age");
    texpect_space();
    texpect_symbol(SYMBOL_GREATER_THAN_OR_EQUAL);
    texpect_space();
    texpect_number("18");
    texpect_code_end();
    texpect_text("\n    Don't forget to register to vote!\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_ELSE);
    texpect_space();
    texpect_keyword(KEYWORD_IF);
    texpect_space();
    texpect_identifier("person.age");
    texpect_space();
    texpect_symbol(SYMBOL_GREATER_THAN_OR_EQUAL);
    texpect_space();
    texpect_number("14");
    texpect_code_end();
    texpect_text(
        "\n    If you'll be 18 before election day, register to vote!\n"
    );
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDIF);
    texpect_code_end();
    texpect_text("\n\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_FOR);
    texpect_space();
    texpect_identifier("person");
    texpect_space();
    texpect_keyword(KEYWORD_IN);
    texpect_space();
    texpect_identifier("people");
    texpect_code_end();
    texpect_text("\n    ");
    texpect_code_start();
    texpect_identifier("person.name");
    texpect_code_end();
    texpect_text(" owes ");
    texpect_code_start();
    texpect_identifier("person.income");
    texpect_space();
    texpect_symbol(SYMBOL_ASTERISK);
    texpect_space();
    texpect_identifier("tax_rate");
    texpect_code_end();
    texpect_text(" this year!\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDFOR);
    texpect_code_end();
    texpect_text("\n\n");
    texpect_text("\n    This is how you would use raw }} and {{ markers.\n");
    texpect_text("\n\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_IF);
    texpect_space();
    texpect_identifier("upper");
    texpect_symbol(SYMBOL_OPAREN);
    texpect_identifier("message");
    texpect_symbol(SYMBOL_CPAREN);
    texpect_space();
    texpect_symbol(SYMBOL_EQUAL);
    texpect_space();
    texpect_string("OR PUT THEM IN STRINGS {{ }} }} {{");
    texpect_code_end();
    texpect_text("\n    You guessed the magic message!\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDIF);
    texpect_code_end();
    texpect_text("\n\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_FOR);
    texpect_space();
    texpect_identifier("fib");
    texpect_space();
    texpect_keyword(KEYWORD_IN);
    texpect_space();
    texpect_symbol(SYMBOL_OBRACKET);
    texpect_number("1");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("1");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("2");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("3");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("4");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("5");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("4");
    texpect_space();
    texpect_symbol(SYMBOL_PLUS);
    texpect_space();
    texpect_number("4");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("13");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("21");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_number("34");
    texpect_symbol(SYMBOL_CBRACKET);
    texpect_code_end();
    texpect_text("\n    ");
    texpect_code_start();
    texpect_keyword(KEYWORD_IF);
    texpect_space();
    texpect_identifier("fib");
    texpect_space();
    texpect_symbol(SYMBOL_EQUAL);
    texpect_space();
    texpect_number("4");
    texpect_code_end();
    texpect_text("\n        ");
    texpect_code_start();
    texpect_keyword(KEYWORD_CONTINUE);
    texpect_code_end();
    texpect_text("\n    ");
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDIF);
    texpect_code_end();
    texpect_text("\n    Double ");
    texpect_code_start();
    texpect_identifier("fib");
    texpect_code_end();
    texpect_text(": ");
    texpect_code_start();
    texpect_identifier("double");
    texpect_symbol(SYMBOL_OPAREN);
    texpect_identifier("fib");
    texpect_symbol(SYMBOL_CPAREN);
    texpect_code_end();
    texpect_text("\n    (also ");
    texpect_code_start();
    texpect_identifier("fibs");
    texpect_symbol(SYMBOL_OBRACKET);
    texpect_identifier("min");
    texpect_symbol(SYMBOL_OPAREN);
    texpect_number("8");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_identifier("max");
    texpect_symbol(SYMBOL_OPAREN);
    texpect_number("0");
    texpect_symbol(SYMBOL_COMMA);
    texpect_space();
    texpect_identifier("fib");
    texpect_space();
    texpect_symbol(SYMBOL_MINUS);
    texpect_space();
    texpect_number("8");
    texpect_symbol(SYMBOL_CPAREN);
    texpect_symbol(SYMBOL_CPAREN);
    texpect_symbol(SYMBOL_CBRACKET);
    texpect_code_end();
    texpect_text(")\n    ");
    texpect_code_start();
    texpect_keyword(KEYWORD_IF);
    texpect_space();
    texpect_identifier("fib");
    texpect_space();
    texpect_symbol(SYMBOL_GREATER_THAN_OR_EQUAL);
    texpect_space();
    texpect_number("10");
    texpect_code_end();
    texpect_text("\n        Whew fibs got too big: ");
    texpect_code_start();
    texpect_identifier("fib");
    texpect_code_end();
    texpect_text("\n        ");
    texpect_code_start();
    texpect_keyword(KEYWORD_BREAK);
    texpect_code_end();
    texpect_text("\n    ");
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDIF);
    texpect_code_end();
    texpect_text("\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_ENDFOR);
    texpect_code_end();
    texpect_text("\n\n");
    texpect_code_start();
    texpect_keyword(KEYWORD_INCLUDE);
    texpect_space();
    texpect_string("/srv/http/templates/footer.txt");
    texpect_code_end();
    texpect_text("\n\nLast little bit down here\n");
}

/* vi: set et ts=4 sw=4: */
