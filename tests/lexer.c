#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"

#include "data.h"

static inline bool lexer_expect(Lexer *lexer, CodeTokenType type,
                                              Status *status) {
    return lexer_load_next(lexer, status) && lexer->code_token.type == type;
}

static inline bool lexer_expect_text(Lexer *lexer, const char *text,
                                                   Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_TEXT, status);
           sslice_equals_cstr(&lexer->code_token.as.text, text);
}

static inline bool lexer_expect_number(Lexer *lexer, const char *number,
                                                     Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_NUMBER, status) &&
           sslice_equals_cstr(&lexer->code_token.as.number, number);
}

static inline bool lexer_expect_string(Lexer *lexer, const char *string,
                                                     Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_STRING, status) &&
           sslice_equals_cstr(&lexer->code_token.as.string, string);
}

static inline bool lexer_expect_keyword(Lexer *lexer, Keyword kw,
                                                      Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_KEYWORD, status) &&
           lexer->code_token.as.keyword == kw;
}

static inline bool lexer_expect_lookup(Lexer *lexer, const char *lookup,
                                                     Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_LOOKUP, status) &&
           sslice_equals_cstr(&lexer->code_token.as.lookup, lookup);
}

static inline bool lexer_expect_function_start(Lexer *lexer,
                                               const char *function_start,
                                               Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_FUNCTION_START, status) &&
           sslice_equals_cstr(&lexer->code_token.as.function, function_start);
}

static inline bool lexer_expect_function_end(Lexer *lexer, Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_FUNCTION_END, status);
}

static inline bool lexer_expect_index_start(Lexer *lexer,
                                            const char *index_start,
                                            Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_INDEX_START, status) &&
           sslice_equals_cstr(&lexer->code_token.as.index, index_start);
}

static inline bool lexer_expect_index_end(Lexer *lexer, Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_INDEX_END, status);
}

static inline bool lexer_expect_array_start(Lexer *lexer, Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_ARRAY_START, status);
}

static inline bool lexer_expect_array_end(Lexer *lexer, Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_ARRAY_END, status);
}

static inline bool lexer_expect_operator(Lexer *lexer, Operator op,
                                                       Status *status) {
    return lexer_expect(lexer, CODE_TOKEN_OPERATOR, status) &&
           lexer->code_token.as.op == op;
}

void test_lexer(void **state) {
    (void)state;
}

/* vi: set et ts=4 sw=4: */
