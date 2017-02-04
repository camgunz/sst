#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "utils.h"

/*
 * I think tokenizer needs to send BLOCK_START and BLOCK_END tokens.
 */

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

#define expected_whitespace(status) status_failure( \
    status,                                         \
    "lexer",                                        \
    LEXER_EXPECTED_WHITESPACE,                      \
    "Expected whitespace"                           \
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

#define unexpected_comma(status) status_failure( \
    status,                                      \
    "lexer",                                     \
    LEXER_UNEXPECTED_COMMA,                      \
    "Unexpected comma"                           \
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
    lexer->code_token.type = CODE_TOKEN_NUMBER;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(&lexer->code_token.as.number, &lexer->tokenizer.token.as.number);
}

static inline
void lexer_handle_string_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_STRING;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(
        &lexer->code_token.as.string,
        &lexer->tokenizer.token.as.string
    );
}

static inline
bool lexer_expect_whitespace(Lexer *lexer, Status *status) {
    bool found_whitespace = false;

    if (!lexer->already_loaded_next) {
        if (!tokenizer_load_next(&lexer->tokenizer, status)) {
            return false;
        }
    }

    while (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
        found_whitespace = true;

        if (!tokenizer_load_next(&lexer->tokenizer, status)) {
            return false;
        }
    }

    lexer->already_loaded_next = true;

    if (!found_whitespace) {
        return expected_whitespace(status);
    }

    return status_ok(status);
}

static inline
bool lexer_skip_whitespace(Lexer *lexer, Status *status) {
    if (!lexer->already_loaded_next) {
        if (!tokenizer_load_next(&lexer->tokenizer, status)) {
            return false;
        }
    }

    while (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
        if (!tokenizer_load_next(&lexer->tokenizer, status)) {
            return false;
        }
    }

    lexer->already_loaded_next = true;

    return status_ok(status);
}

static inline
bool lexer_handle_keyword_token(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_KEYWORD;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.keyword = lexer->tokenizer.token.as.keyword;
    return lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_operator(Lexer *lexer, Operator op, Status *status) {
    lexer->code_token.type = CODE_TOKEN_OPERATOR;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.op = op;

    if (op == OP_OPAREN) {
        return lexer_push_state(lexer, LEXER_STATE_PAREN, status);
    }

    if (op == OP_CPAREN) {
        return lexer_pop_state(lexer, status);
    }

    return lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_lookup(Lexer *lexer, SSlice *lookup, Status *status) {
    lexer->code_token.type = CODE_TOKEN_LOOKUP;
    lexer->code_token.location = lookup->data;
    sslice_copy(&lexer->code_token.as.lookup, lookup);

    return lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_function(Lexer *lexer, SSlice *function,
                                            Status *status) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_START;
    lexer->code_token.location = function->data;
    sslice_copy(&lexer->code_token.as.function, function);

    return lexer_push_state(lexer, LEXER_STATE_FUNCTION, status);
}

static inline
bool lexer_set_token_function_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status) &&
           lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_function_argument_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_ARGUMENT_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_array(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_START;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_push_state(lexer, LEXER_STATE_ARRAY, status);
}

static inline
bool lexer_set_token_array_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status) &&
           lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_array_element_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_ELEMENT_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_skip_whitespace(lexer, status);
}

static inline
bool lexer_set_token_index(Lexer *lexer, SSlice *index,
                                         Status *status) {
    lexer->code_token.type = CODE_TOKEN_INDEX_START;
    lexer->code_token.location = index->data;
    sslice_copy(&lexer->code_token.as.index, index);

    return lexer_push_state(lexer, LEXER_STATE_INDEX, status);
}

static inline
bool lexer_set_token_index_end(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_INDEX_END;
    lexer->code_token.location = lexer->tokenizer.token.location;

    return lexer_pop_state(lexer, status) &&
           lexer_skip_whitespace(lexer, status);
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
        case SYMBOL_EXCLAMATION_POINT:
            return lexer_set_token_operator(
                lexer,
                OP_BOOL_NOT,
                status
            );
        case SYMBOL_COMMA:
            if (lexer_check_state(lexer, LEXER_STATE_FUNCTION)) {
                lexer_set_token_function_argument_end(lexer, status);
                return status_ok(status);
            }

            if (lexer_check_state(lexer, LEXER_STATE_ARRAY)) {
                lexer_set_token_array_element_end(lexer, status);
                return status_ok(status);
            }

            return unexpected_comma(status);
        case SYMBOL_PLUS:
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }

            lexer->already_loaded_next = true;

            if (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
                return lexer_set_token_operator(lexer, OP_MATH_ADD, status);
            }

            return lexer_set_token_operator(lexer, OP_MATH_POSITIVE, status);
        case SYMBOL_MINUS:
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }

            lexer->already_loaded_next = true;

            if (lexer->tokenizer.token.type == TOKEN_WHITESPACE) {
                return lexer_set_token_operator(
                    lexer,
                    OP_MATH_SUBTRACT,
                    status
                );
            }

            return lexer_set_token_operator(lexer, OP_MATH_NEGATIVE, status);
        case SYMBOL_ASTERISK:
            return lexer_set_token_operator(lexer, OP_MATH_MULTIPLY, status);
        case SYMBOL_FORWARD_SLASH:
            return lexer_set_token_operator(lexer, OP_MATH_MULTIPLY, status);
        case SYMBOL_PERCENT:
            return lexer_set_token_operator(lexer, OP_MATH_REMAINDER, status);
        case SYMBOL_CARET:
            return lexer_set_token_operator(lexer, OP_MATH_EXPONENT, status);
        case SYMBOL_AND:
            return lexer_set_token_operator(lexer, OP_BOOL_AND, status);
        case SYMBOL_OR:
            return lexer_set_token_operator(lexer, OP_BOOL_OR, status);
        case SYMBOL_EQUAL:
            return lexer_set_token_operator(lexer, OP_BOOL_EQUAL, status);
        case SYMBOL_NOT_EQUAL:
            return lexer_set_token_operator(lexer, OP_BOOL_NOT_EQUAL, status);
        case SYMBOL_LESS_THAN_OR_EQUAL:
            return lexer_set_token_operator(
                lexer,
                OP_BOOL_LESS_THAN_OR_EQUAL,
                status
            );
        case SYMBOL_LESS_THAN:
            return lexer_set_token_operator(lexer, OP_BOOL_LESS_THAN, status);
        case SYMBOL_GREATER_THAN_OR_EQUAL:
            return lexer_set_token_operator(
                lexer,
                OP_BOOL_GREATER_THAN_OR_EQUAL,
                status
            );
        case SYMBOL_GREATER_THAN:
            return lexer_set_token_operator(
                lexer,
                OP_BOOL_GREATER_THAN,
                status
            );
        default:
            return unexpected_symbol(status);
    }

    return status_ok(status);
}

static
bool lexer_handle_identifier_token(Lexer *lexer, Status *status) {
    SSlice identifier;

    sslice_copy(&identifier, &lexer->tokenizer.token.as.identifier);

    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    if (lexer->tokenizer.token.type == TOKEN_SYMBOL) {
        if (lexer->tokenizer.token.as.symbol == SYMBOL_OPAREN) {
            return lexer_set_token_function(lexer, &identifier, status);
        }

        if (lexer->tokenizer.token.as.symbol == SYMBOL_OBRACKET) {
            return lexer_set_token_index(lexer, &identifier, status);
        }

        if (lexer->tokenizer.token.as.symbol != SYMBOL_CPAREN) {
            return invalid_syntax(status);
        }
    }

    lexer->already_loaded_next = true;

    return lexer_set_token_lookup(lexer, &identifier, status);
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
    }

    lexer->already_loaded_next = false;

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
            return lexer_handle_keyword_token(lexer, status);
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
