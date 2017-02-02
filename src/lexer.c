#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "utils.h"

#define INITIAL_STATE_ALLOC 16

#define invalid_syntax(status) status_failure( \
    status,                                    \
    "lexer",                                   \
    LEXER_INVALID_STATUS,                      \
    "Invalid syntax"                           \
)

#define unknown_token(status) status_failure( \
    status,                                   \
    "lexer",                                  \
    LEXER_UNKNOWN_TOKEN,                      \
    "Unknown token"                           \
)

#define unexpected_whitespace(status) status_failure( \
    status,                                           \
    "lexer",                                          \
    LEXER_UNEXPECTED_WHITESPACE,                      \
    "Unexpected whitespace"                           \
)

#define unexpected_symbol(status) status_failure( \
    status,                                       \
    "lexer",                                      \
    LEXER_UNEXPECTED_SYMBOL,                      \
    "Unexpected symbol"                           \
)

#define unexpected_token(status) status_failure( \
    status,                                      \
    "lexer",                                     \
    LEXER_UNEXPECTED_TOKEN,                      \
    "Unexpected token"                           \
)

#define expected_number(status) status_failure( \
    status,                                     \
    "lexer",                                    \
    LEXER_EXPECTED_NUMBER,                      \
    "Expected number"                           \
)

#define unexpected_cbracket(status) status_failure( \
    status,                                         \
    "lexer",                                        \
    LEXER_UNEXPECTED_CLOSE_BRACKET,                 \
    "Unexpected close bracket"                      \
)

static inline
bool lexer_push_state(Lexer *lexer, LexerState state, Status *status) {
    LexerState *new_state = NULL;

    if (!array_append(&lexer->states, (void **)&new_state, status)) {
        return false;
    }

    *new_state = state;

    return status_ok(status);
}

static inline
bool lexer_pop_state(Lexer *lexer, Status *status) {
    return array_delete(&lexer->states, lexer->states.len - 1, status);
}

static inline
bool lexer_check_state(Lexer *lexer, LexerState state) {
    if (lexer->states.len < 1) {
        return false;
    }

    LexerState *lexer_state = array_index_fast(
        &lexer->states, lexer->states.len - 1
    );

    return state == *lexer_state;
}

static inline
void lexer_handle_text_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_TEXT;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(&lexer->code_token.as.text, &lexer->tokenizer.token.as.text);
}

static inline
void lexer_handle_number_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_TEXT;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(&lexer->code_token.as.number, &lexer->tokenizer.token.as.number);
}

static inline
void lexer_handle_string_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_TEXT;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(&lexer->code_token.as.string, &lexer->tokenizer.token.as.string);
}

static inline
void lexer_handle_keyword_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_TEXT;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.keyword = lexer->tokenizer.token.as.keyword;
}

static inline
bool lexer_set_token_operator(Lexer *lexer, Operator op, Status *status) {
    lexer->code_token.type = CODE_TOKEN_OPERATOR;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.op = OP_OPAREN;

    if (op == OP_OPAREN) {
        return lexer_push_state(lexer, LEXER_STATE_PAREN, status);
    }

    if (op == OP_CPAREN) {
        return lexer_pop_state(lexer, status);
    }

    return status_ok(status);
}

static inline
bool lexer_set_token_unary_math_operator(Lexer *lexer, Operator op,
                                                       Status *status) {
    const char *previous_location = lexer->tokenizer.token.location;

    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    /*
     * [FIXME] This needs to accept SYMBOL_OPAREN as well, but the lexer has to
     *         look two tokens ahead instead of just one, and the tokenizer
     *         won't currently support that.
     */
    if (lexer->tokenizer.token.type != TOKEN_NUMBER) {
        return expected_number(status);
    }

    lexer->code_token.type = CODE_TOKEN_OPERATOR;
    lexer->code_token.location = previous_location;
    lexer->code_token.as.op = op;

    lexer->already_loaded_next = true;

    return status_ok(status);
}

static inline
bool lexer_set_token_unary_boolean_operator(Lexer *lexer, Operator op,
                                                          Status *status) {
    const char *previous_location = lexer->tokenizer.token.location;

    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    /* 'true' and 'false' are special identifiers */
    if (lexer->tokenizer.token.type != TOKEN_IDENTIFIER) {
        return expected_number(status);
    }

    lexer->code_token.type = CODE_TOKEN_OPERATOR;
    lexer->code_token.location = previous_location;
    lexer->code_token.as.op = op;

    lexer->already_loaded_next = true;

    return status_ok(status);
}

static inline
void lexer_set_token_lookup(Lexer *lexer, SSlice *lookup,
                                          const char *location) {
    lexer->code_token.type = CODE_TOKEN_LOOKUP;
    lexer->code_token.location = location;
    sslice_copy(&lexer->code_token.as.lookup, lookup);

    lexer->already_loaded_next = true;
}

static inline
bool lexer_set_token_function(Lexer *lexer, SSlice *function,
                                            const char *location,
                                            Status *status) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_START;
    lexer->code_token.location = location;
    sslice_copy(&lexer->code_token.as.function, function);

    return lexer_push_state(lexer, LEXER_STATE_FUNCTION, status);
}

static inline
bool lexer_set_token_function_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status);
}

static inline
bool lexer_set_token_index_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_INDEX_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status);
}

static inline
bool lexer_set_token_array_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status);
}

static inline
bool lexer_set_token_index(Lexer *lexer, SSlice *index,
                                         const char *location,
                                         Status *status) {
    lexer->code_token.type = CODE_TOKEN_INDEX_START;
    lexer->code_token.location = location;
    sslice_copy(&lexer->code_token.as.index, index);

    return lexer_push_state(lexer, LEXER_STATE_INDEX, status);
}

static inline
bool lexer_set_token_array(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_START;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_push_state(lexer, LEXER_STATE_ARRAY, status);
}

static
bool lexer_handle_symbol_token(Lexer *lexer, Status *status) {
    switch (lexer->tokenizer.token.as.symbol) {
        case SYMBOL_OPAREN:
            return lexer_set_token_operator(lexer, OP_OPAREN, status);
        case SYMBOL_CPAREN:
            if (lexer_check_state(lexer, LEXER_STATE_FUNCTION)) {
                return lexer_set_token_function_end(lexer, status);
            }

            return lexer_set_token_operator(lexer, OP_CPAREN, status);
        case SYMBOL_OBRACKET:
            return lexer_set_token_array(lexer, status);
        case SYMBOL_CBRACKET:
            if (lexer_check_state(lexer, LEXER_STATE_INDEX)) {
                return lexer_set_token_index_end(lexer, status);
            }

            if (lexer_check_state(lexer, LEXER_STATE_ARRAY)) {
                return lexer_set_token_array_end(lexer, status);
            }

            return unexpected_cbracket(status);
        case SYMBOL_PLUS:
            return lexer_set_token_unary_math_operator(
                lexer,
                OP_MATH_POSITIVE,
                status
            );
        case SYMBOL_MINUS:
            return lexer_set_token_unary_math_operator(
                lexer,
                OP_MATH_NEGATIVE,
                status
            );
        case SYMBOL_EXCLAMATION_POINT:
            return lexer_set_token_unary_boolean_operator(
                lexer,
                OP_BOOL_NOT,
                status
            );
        default:
            return unexpected_symbol(status);
    }

    return status_ok(status);
}

static
bool lexer_handle_identifier_token(Lexer *lexer, Status *status) {
    Token token;

    memcpy(&token, &lexer->tokenizer.token, sizeof(Token));

    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    if (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
        while (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }
        }

        lexer_set_token_lookup(lexer, &token.as.identifier, token.location);
        return status_ok(status);
    }

    if (lexer->tokenizer.token.as.symbol == SYMBOL_OPAREN) {
        return lexer_set_token_function(
            lexer,
            &token.as.identifier,
            token.location,
            status
        );
    }

    if (lexer->tokenizer.token.as.symbol == SYMBOL_OBRACKET) {
        return lexer_set_token_index(
            lexer,
            &token.as.identifier,
            token.location,
            status
        );
    }

    return unexpected_token(status);
}

void lexer_clear(Lexer *lexer) {
    tokenizer_clear(&lexer->tokenizer);
    lexer->code_token.type = CODE_TOKEN_UNKNOWN;
    lexer->code_token.location = NULL;
    lexer->already_loaded_next = false;
    array_clear(&lexer->states);
}

void lexer_set_data(Lexer *lexer, SSlice *sslice) {
    tokenizer_init(&lexer->tokenizer, sslice);
    lexer->code_token.type = CODE_TOKEN_UNKNOWN;
    lexer->code_token.location = NULL;
    lexer->already_loaded_next = false;
    array_clear(&lexer->states);
}

bool lexer_init(Lexer *lexer, SSlice *data, Status *status) {
    tokenizer_init(&lexer->tokenizer, data);
    lexer->code_token.type = CODE_TOKEN_UNKNOWN;
    lexer->code_token.location = NULL;
    lexer->already_loaded_next = false;

    if (!array_init_alloc(&lexer->states, sizeof(LexerState),
                                          INITIAL_STATE_ALLOC,
                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool lexer_load_next(Lexer *lexer, Status *status) {
    if (!lexer->already_loaded_next) {
        if (!tokenizer_load_next(&lexer->tokenizer, status)) {
            return false;
        }

        lexer->already_loaded_next = false;
    }

    switch (lexer->tokenizer.token.type) {
        case TOKEN_UNKNOWN:
            return unknown_token(status);
        case TOKEN_TEXT:
            lexer_handle_text_token(lexer);
            return status_ok(status);
        case TOKEN_NUMBER:
            lexer_handle_number_token(lexer);
            return status_ok(status);
        case TOKEN_STRING:
            lexer_handle_string_token(lexer);
            return status_ok(status);
        case TOKEN_SYMBOL:
            return lexer_handle_symbol_token(lexer, status);
        case TOKEN_KEYWORD:
            lexer_handle_keyword_token(lexer);
            return status_ok(status);
        case TOKEN_IDENTIFIER:
            return lexer_handle_identifier_token(lexer, status);
        case TOKEN_WHITESPACE:
            return unexpected_whitespace(status);
        default:
            break;
    }

    return unexpected_token(status);
}

/* vi: set et ts=4 sw=4: */
