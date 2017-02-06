#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"

#include "data.h"

#define lexpect(token_type) \
    if (!lexer_load_next(&lexer, &status)) { \
        printf("lexer_load_next_error: %s\n", status.message); \
        assert_true(false); \
    } \
    assert_int_equal(lexer.code_token.type, token_type)

#define lexpect_text(_text) \
    lexpect(CODE_TOKEN_TEXT); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.text, _text))

#define lexpect_number(_number) \
    lexpect(CODE_TOKEN_NUMBER); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.number, _number))

#define lexpect_string(_string) \
    lexpect(CODE_TOKEN_STRING); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.string, _string))

#define lexpect_keyword(kw) \
    lexpect(CODE_TOKEN_KEYWORD); \
    assert_int_equal(lexer.code_token.as.keyword, kw)

#define lexpect_lookup(_lookup) \
    lexpect(CODE_TOKEN_LOOKUP); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.lookup, _lookup))

#define lexpect_function_start(fstart) \
    lexpect(CODE_TOKEN_FUNCTION_START); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.function, fstart))

#define lexpect_function_end(lexer) \
    lexpect(CODE_TOKEN_FUNCTION_END)

#define lexpect_function_argument_end(lexer) \
    lexpect(CODE_TOKEN_FUNCTION_ARGUMENT_END)

#define lexpect_index_start(index_start) \
    lexpect(CODE_TOKEN_INDEX_START); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.index, index_start))

#define lexpect_index_end(lexer) \
    lexpect(CODE_TOKEN_INDEX_END)

#define lexpect_array_start(lexer) \
    lexpect(CODE_TOKEN_ARRAY_START)

#define lexpect_array_end(lexer) \
    lexpect(CODE_TOKEN_ARRAY_END)

#define lexpect_array_element_end(lexer) \
    lexpect(CODE_TOKEN_ARRAY_ELEMENT_END)

#define lexpect_operator(_op) \
    lexpect(CODE_TOKEN_OPERATOR); \
    assert_int_equal(lexer.code_token.as.op, _op)

void test_lexer(void **state) {
    String s;
    SSlice ss;
    Status status;
    String buf;
    Lexer lexer;

    (void)state;

    status_init(&status);

    assert_true(string_init(&buf, "", &status));

    assert_true(string_init(&s, TEMPLATE, &status));
    assert_true(string_slice(&s, 0, s.len, &ss, &status));

    assert_true(lexer_init(&lexer, &ss, &status));
    lexpect_keyword(KEYWORD_INCLUDE);
    lexpect_string("/srv/http/templates/header.txt");
    lexpect_text("\n\n");
    lexpect_keyword(KEYWORD_IF);
    lexpect_lookup("person.age");
    lexpect_operator(OP_BOOL_GREATER_THAN_OR_EQUAL);
    lexpect_number("18");
    lexpect_text("\n...\n");
    lexpect_keyword(KEYWORD_ENDIF);
    lexpect_text("\n\n");
    lexpect_keyword(KEYWORD_FOR);
    lexpect_lookup("person");
    lexpect_keyword(KEYWORD_IN);
    lexpect_lookup("people");
    lexpect_text("\n    ");
    lexpect_lookup("person.name");
    lexpect_text(" owes ");
    lexpect_lookup("person.income");
    lexpect_operator(OP_MATH_MULTIPLY);
    lexpect_lookup("tax_rate");
    lexpect_text(" this year!\n");
    lexpect_keyword(KEYWORD_ENDFOR);
    lexpect_text("\n\n");
    lexpect_text("\n    This is how you would use raw }} and {{ markers.\n");
    lexpect_text("\n");
    lexpect_keyword(KEYWORD_IF);
    lexpect_function_start("upper");
    lexpect_lookup("message");
    lexpect_function_end(lexer);
    lexpect_operator(OP_BOOL_EQUAL);
    lexpect_string("OR PUT THEM IN STRINGS {{ }} }} {{");
    lexpect_text("\nYou guessed the magic message!\n");
    lexpect_keyword(KEYWORD_ENDIF);
    lexpect_text("\n\n");
    lexpect_keyword(KEYWORD_FOR);
    lexpect_lookup("fib");
    lexpect_keyword(KEYWORD_IN);
    lexpect_array_start(lexer);
    lexpect_number("1");
    lexpect_array_element_end(lexer);
    lexpect_number("1");
    lexpect_array_element_end(lexer);
    lexpect_number("2");
    lexpect_array_element_end(lexer);
    lexpect_number("3");
    lexpect_array_element_end(lexer);
    lexpect_number("5");
    lexpect_array_element_end(lexer);
    lexpect_number("8");
    lexpect_array_end(lexer);
    lexpect_text("\n    Double ");
    lexpect_lookup("fib");
    lexpect_text(": ");
    lexpect_function_start("double");
    lexpect_lookup("fib");
    lexpect_function_end(lexer);
    lexpect_text("\n");
    lexpect_keyword(KEYWORD_ENDFOR);
    lexpect_text("\n");
    lexpect_keyword(KEYWORD_INCLUDE);
    lexpect_string("/srv/http/templates/footer.txt");
    lexpect_text("\nLast little bit down here\n");
}

/* vi: set et ts=4 sw=4: */
