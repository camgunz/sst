#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"

#include "data.h"

#define lexer_expect(lexer, token_type, status) \
    if (!lexer_load_next(&lexer, status)) { \
        printf("lexer_load_next_error: %s\n", (status)->message); \
        assert_true(false); \
    } \
    assert_int_equal(lexer.code_token.type, token_type)

#define lexer_expect_text(lexer, _text, status) \
    lexer_expect(lexer, CODE_TOKEN_TEXT, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.text, _text))

#define lexer_expect_number(lexer, _number, status) \
    lexer_expect(lexer, CODE_TOKEN_NUMBER, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.number, _number))

#define lexer_expect_string(lexer, _string, status) \
    lexer_expect(lexer, CODE_TOKEN_STRING, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.string, _string))

#define lexer_expect_keyword(lexer, kw, status) \
    lexer_expect(lexer, CODE_TOKEN_KEYWORD, status); \
    assert_int_equal(lexer.code_token.as.keyword, kw)

#define lexer_expect_lookup(lexer, _lookup, status) \
    lexer_expect(lexer, CODE_TOKEN_LOOKUP, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.lookup, _lookup))

#define lexer_expect_function_start(lexer, fstart, status) \
    lexer_expect(lexer, CODE_TOKEN_FUNCTION_START, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.function, fstart))

#define lexer_expect_function_end(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_FUNCTION_END, status)

#define lexer_expect_function_argument_end(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_FUNCTION_ARGUMENT_END, status)

#define lexer_expect_index_start(lexer, index_start, status) \
    lexer_expect(lexer, CODE_TOKEN_INDEX_START, status); \
    assert_true(sslice_equals_cstr(&lexer.code_token.as.index, index_start))

#define lexer_expect_index_end(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_INDEX_END, status)

#define lexer_expect_array_start(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_ARRAY_START, status)

#define lexer_expect_array_end(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_ARRAY_END, status)

#define lexer_expect_array_element_end(lexer, status) \
    lexer_expect(lexer, CODE_TOKEN_ARRAY_ELEMENT_END, status)

#define lexer_expect_operator(lexer, _op, status) \
    lexer_expect(lexer, CODE_TOKEN_OPERATOR, status); \
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
    lexer_expect_keyword(lexer, KEYWORD_INCLUDE, &status);
    lexer_expect_string(lexer, "/srv/http/templates/header.txt", &status);
    lexer_expect_text(lexer, "\n\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_IF, &status);
    lexer_expect_lookup(lexer, "person.age", &status);
    lexer_expect_operator(lexer, OP_BOOL_GREATER_THAN_OR_EQUAL, &status);
    lexer_expect_number(lexer, "18", &status);
    lexer_expect_text(lexer, "\n...\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_ENDIF, &status);
    lexer_expect_text(lexer, "\n\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_FOR, &status);
    lexer_expect_lookup(lexer, "person", &status);
    lexer_expect_keyword(lexer, KEYWORD_IN, &status);
    lexer_expect_lookup(lexer, "people", &status);
    lexer_expect_text(lexer, "\n    ", &status);
    lexer_expect_lookup(lexer, "person.name", &status);
    lexer_expect_text(lexer, " owes ", &status);
    lexer_expect_lookup(lexer, "person.income", &status);
    lexer_expect_operator(lexer, OP_MATH_MULTIPLY, &status);
    lexer_expect_lookup(lexer, "tax_rate", &status);
    lexer_expect_text(lexer, " this year!\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_ENDFOR, &status);
    lexer_expect_text(lexer, "\n\n", &status);
    lexer_expect_text(
        lexer,
        "\n    This is how you would use raw }} and {{ markers.\n",
        &status
    );
    lexer_expect_text(lexer, "\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_IF, &status);
    lexer_expect_function_start(lexer, "upper", &status);
    lexer_expect_lookup(lexer, "message", &status);
    lexer_expect_function_end(lexer, &status);
    lexer_expect_operator(lexer, OP_BOOL_EQUAL, &status);
    lexer_expect_string(lexer, "OR PUT THEM IN STRINGS {{ }} }} {{", &status);
    lexer_expect_text(lexer, "\nYou guessed the magic message!\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_ENDIF, &status);
    lexer_expect_text(lexer, "\n\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_FOR, &status);
    lexer_expect_lookup(lexer, "fib", &status);
    lexer_expect_keyword(lexer, KEYWORD_IN, &status);
    lexer_expect_array_start(lexer, &status);
    lexer_expect_number(lexer, "1", &status);
    lexer_expect_array_element_end(lexer, &status);
    lexer_expect_number(lexer, "1", &status);
    lexer_expect_array_element_end(lexer, &status);
    lexer_expect_number(lexer, "2", &status);
    lexer_expect_array_element_end(lexer, &status);
    lexer_expect_number(lexer, "3", &status);
    lexer_expect_array_element_end(lexer, &status);
    lexer_expect_number(lexer, "5", &status);
    lexer_expect_array_element_end(lexer, &status);
    lexer_expect_number(lexer, "8", &status);
    lexer_expect_array_end(lexer, &status);
    lexer_expect_text(lexer, "\n    Double ", &status);
    lexer_expect_lookup(lexer, "fib", &status);
    lexer_expect_text(lexer, ": ", &status);
    lexer_expect_function_start(lexer, "double", &status);
    lexer_expect_lookup(lexer, "fib", &status);
    lexer_expect_function_end(lexer, &status);
    lexer_expect_text(lexer, "\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_ENDFOR, &status);
    lexer_expect_text(lexer, "\n", &status);
    lexer_expect_keyword(lexer, KEYWORD_INCLUDE, &status);
    lexer_expect_string(lexer, "/srv/http/templates/footer.txt", &status);
    lexer_expect_text(lexer, "\nLast little bit down here\n", &status);
}

/* vi: set et ts=4 sw=4: */
