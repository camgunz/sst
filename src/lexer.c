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

#define unexpected_space(status) status_failure( \
    status,                                      \
    "lexer",                                     \
    LEXER_UNEXPECTED_SPACE,                      \
    "Unexpected space"                           \
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

#define unexpected_comma(status) status_failure( \
    status,                                      \
    "lexer",                                     \
    LEXER_UNEXPECTED_COMMA,                      \
    "Unexpected comma"                           \
)

#define unexpected_cparen(status) status_failure( \
    status,                                       \
    "lexer",                                      \
    LEXER_UNEXPECTED_CLOSE_PAREN,                 \
    "Unexpected close paren"                      \
)

#define unexpected_cbracket(status) status_failure( \
    status,                                         \
    "lexer",                                        \
    LEXER_UNEXPECTED_CLOSE_BRACKET,                 \
    "Unexpected close bracket"                      \
)

#define unexpected_code_end(status) status_failure( \
    status,                                         \
    "lexer",                                        \
    LEXER_UNEXPECTED_CODE_END,                      \
    "Unexpected code end"                           \
)

#define expected_space(status) status_failure( \
    status,                                    \
    "lexer",                                   \
    LEXER_EXPECTED_SPACE,                      \
    "Expected space"                           \
)

#define expected_space_or_close_paren(status) status_failure( \
    status,                                                   \
    "lexer",                                                  \
    LEXER_EXPECTED_SPACE_OR_CLOSE_PAREN,                      \
    "Expected space or close paren"                           \
)

#define expected_space_or_close_bracket(status) status_failure( \
    status,                                                     \
    "lexer",                                                    \
    LEXER_EXPECTED_SPACE_OR_CLOSE_BRACKET,                      \
    "Expected space or close bracket"                           \
)

#define expected_space_or_expression_end(status) status_failure( \
    status,                                                      \
    "lexer",                                                     \
    LEXER_EXPECTED_SPACE_OR_EXPRESSION_END,                      \
    "Expected space or code end"                                 \
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
LexerState lexer_get_state(Lexer *lexer) {
    if (lexer->states.len == 0) {
        return LEXER_STATE_NORMAL;
    }

    return *(LexerState *)array_index_fast(
        &lexer->states,
        lexer->states.len - 1
    );
}

static inline
bool lexer_check_state(Lexer *lexer, LexerState state) {
    return lexer_get_state(lexer) == state;
}

static inline
bool lexer_expect_space(Lexer *lexer, Status *status) {
    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    if (lexer->tokenizer.token.type != TOKEN_SPACE) {
        return expected_space(status);
    }

    return status_ok(status);
}

static inline
bool lexer_expect_space_or_expression_end(Lexer *lexer, Status *status) {
    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    switch (lexer->tokenizer.token.type) {
        case TOKEN_CODE_END:
        case TOKEN_SPACE:
            return status_ok(status);
        case TOKEN_SYMBOL:
            switch (lexer->tokenizer.token.as.symbol) {
                case SYMBOL_COMMA:
                case SYMBOL_CPAREN:
                case SYMBOL_CBRACKET:
                    lexer->already_loaded_next = true;
                    return status_ok(status);
                default:
                    break;
            }
        default:
            return expected_space_or_expression_end(status);
    }
}

static inline
bool lexer_expect_not_space_and_not_code_end(Lexer *lexer, Status *status) {
    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    switch (lexer->tokenizer.token.type) {
        case TOKEN_SPACE:
            return unexpected_space(status);
        case TOKEN_CODE_END:
            return unexpected_code_end(status);
        default:
            lexer->already_loaded_next = true;
            return status_ok(status);
    }
}

static inline
bool lexer_expect_space_or_cparen(Lexer *lexer, Status *status) {
    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    switch (lexer->tokenizer.token.type) {
        case TOKEN_SPACE:
            return status_ok(status);
        case TOKEN_SYMBOL:
            switch (lexer->tokenizer.token.as.symbol) {
                case SYMBOL_CPAREN:
                    return status_ok(status);
                default:
                    break;
            }
        default:
            break;
    }

    return expected_space_or_close_paren(status);
}

static inline
bool lexer_expect_space_or_cbracket(Lexer *lexer, Status *status) {
    if (!tokenizer_load_next(&lexer->tokenizer, status)) {
        return false;
    }

    switch (lexer->tokenizer.token.type) {
        case TOKEN_SPACE:
            return status_ok(status);
        case TOKEN_SYMBOL:
            switch (lexer->tokenizer.token.as.symbol) {
                case SYMBOL_CBRACKET:
                    return status_ok(status);
                default:
                    break;
            }
        default:
            break;
    }

    return expected_space_or_close_paren(status);
}

static inline
void lexer_handle_text_token(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_TEXT;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(&lexer->code_token.as.text, &lexer->tokenizer.token.as.text);
}

static inline
bool lexer_handle_number_token(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_NUMBER;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(
        &lexer->code_token.as.number,
        &lexer->tokenizer.token.as.number
    );

    return lexer_expect_space_or_expression_end(lexer, status);
}

static inline
bool lexer_handle_string_token(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_STRING;
    lexer->code_token.location = lexer->tokenizer.token.location;
    sslice_copy(
        &lexer->code_token.as.string,
        &lexer->tokenizer.token.as.string
    );

    return lexer_expect_space_or_expression_end(lexer, status);
}

static inline
bool lexer_handle_keyword_token(Lexer *lexer, Status *status) {
    lexer->code_token.type = CODE_TOKEN_KEYWORD;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.keyword = lexer->tokenizer.token.as.keyword;

    return lexer_expect_space_or_expression_end(lexer, status);
}

static inline
void lexer_set_token_operator(Lexer *lexer, Operator op) {
    lexer->code_token.type = CODE_TOKEN_OPERATOR;
    lexer->code_token.location = lexer->tokenizer.token.location;
    lexer->code_token.as.op = op;
}

static inline
void lexer_set_token_lookup(Lexer *lexer, SSlice *lookup) {
    lexer->code_token.type = CODE_TOKEN_LOOKUP;
    lexer->code_token.location = lookup->data;
    sslice_copy(&lexer->code_token.as.lookup, lookup);
}

static inline
void lexer_set_token_function(Lexer *lexer, SSlice *function) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_START;
    lexer->code_token.location = function->data;
    sslice_copy(&lexer->code_token.as.function, function);
}

static inline
void lexer_set_token_function_end(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_END;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static inline
void lexer_set_token_function_argument_end(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_FUNCTION_ARGUMENT_END;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static inline
void lexer_set_token_array(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_START;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static inline
void lexer_set_token_array_end(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_END;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static inline
void lexer_set_token_array_element_end(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_ARRAY_ELEMENT_END;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static inline
void lexer_set_token_index(Lexer *lexer, SSlice *index) {
    lexer->code_token.type = CODE_TOKEN_INDEX_START;
    lexer->code_token.location = index->data;
    sslice_copy(&lexer->code_token.as.index, index);
}

static inline
void lexer_set_token_index_end(Lexer *lexer) {
    lexer->code_token.type = CODE_TOKEN_INDEX_END;
    lexer->code_token.location = lexer->tokenizer.token.location;
}

static
bool lexer_handle_symbol_token(Lexer *lexer, Status *status) {
    switch (lexer->tokenizer.token.as.symbol) {
        case SYMBOL_OPAREN:
            lexer_set_token_operator(lexer, OP_OPAREN);
            return lexer_push_state(lexer, LEXER_STATE_PAREN, status) &&
                   lexer_expect_not_space_and_not_code_end(lexer, status);
        case SYMBOL_CPAREN:
            switch (lexer_get_state(lexer)) {
                case LEXER_STATE_FUNCTION:
                    lexer_set_token_function_end(lexer);
                    break;
                case LEXER_STATE_PAREN:
                    lexer_set_token_operator(lexer, OP_CPAREN);
                    break;
                default:
                    return unexpected_cparen(status);
            }

            return lexer_pop_state(lexer, status) &&
                   lexer_expect_space_or_expression_end(lexer, status);
        case SYMBOL_OBRACKET:
            lexer_set_token_array(lexer);
            return lexer_push_state(lexer, LEXER_STATE_ARRAY, status) &&
                   lexer_expect_not_space_and_not_code_end(lexer, status);
        case SYMBOL_CBRACKET:
            switch (lexer_get_state(lexer)) {
                case LEXER_STATE_INDEX:
                    lexer_set_token_index_end(lexer);
                    break;
                case LEXER_STATE_ARRAY:
                    lexer_set_token_array_end(lexer);
                    break;
                default:
                    return unexpected_cbracket(status);
            }

            return lexer_expect_space_or_expression_end(lexer, status) &&
                   lexer_expect_not_space_and_not_code_end(lexer, status);
        case SYMBOL_EXCLAMATION_POINT:
            lexer_set_token_operator(lexer, OP_BOOL_NOT);
            return lexer_expect_not_space_and_not_code_end(lexer, status);
        case SYMBOL_COMMA:
            switch (lexer_get_state(lexer)) {
                case LEXER_STATE_FUNCTION:
                    lexer_set_token_function_argument_end(lexer);
                    return lexer_expect_space_or_cparen(lexer, status);
                case LEXER_STATE_ARRAY:
                    lexer_set_token_array_element_end(lexer);
                    return lexer_expect_space_or_cbracket(lexer, status);
                default:
                    return unexpected_comma(status);
            }
        case SYMBOL_PLUS:
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }

            if (lexer->tokenizer.token.type == TOKEN_SPACE) {
                lexer_set_token_operator(lexer, OP_MATH_ADD);
            }
            else {
                lexer->already_loaded_next = true;
                lexer_set_token_operator(lexer, OP_MATH_POSITIVE);
            }

            return status_ok(status);
        case SYMBOL_MINUS:
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }

            if (lexer->tokenizer.token.type == TOKEN_SPACE) {
                lexer_set_token_operator(lexer, OP_MATH_SUBTRACT);
            }
            else {
                lexer->already_loaded_next = true;
                lexer_set_token_operator(lexer, OP_MATH_NEGATIVE);
            }

            return status_ok(status);
        case SYMBOL_ASTERISK:
            lexer_set_token_operator(lexer, OP_MATH_MULTIPLY);
            return lexer_expect_space(lexer, status);
        case SYMBOL_FORWARD_SLASH:
            lexer_set_token_operator(lexer, OP_MATH_MULTIPLY);
            return lexer_expect_space(lexer, status);
        case SYMBOL_PERCENT:
            lexer_set_token_operator(lexer, OP_MATH_REMAINDER);
            return lexer_expect_space(lexer, status);
        case SYMBOL_CARET:
            lexer_set_token_operator(lexer, OP_MATH_EXPONENT);
            return lexer_expect_space(lexer, status);
        case SYMBOL_AND:
            lexer_set_token_operator(lexer, OP_BOOL_AND);
            return lexer_expect_space(lexer, status);
        case SYMBOL_OR:
            lexer_set_token_operator(lexer, OP_BOOL_OR);
            return lexer_expect_space(lexer, status);
        case SYMBOL_EQUAL:
            lexer_set_token_operator(lexer, OP_BOOL_EQUAL);
            return lexer_expect_space(lexer, status);
        case SYMBOL_NOT_EQUAL:
            lexer_set_token_operator(lexer, OP_BOOL_NOT_EQUAL);
            return lexer_expect_space(lexer, status);
        case SYMBOL_LESS_THAN_OR_EQUAL:
            lexer_set_token_operator(lexer, OP_BOOL_LESS_THAN_OR_EQUAL);
            return lexer_expect_space(lexer, status);
        case SYMBOL_LESS_THAN:
            lexer_set_token_operator(lexer, OP_BOOL_LESS_THAN);
            return lexer_expect_space(lexer, status);
        case SYMBOL_GREATER_THAN_OR_EQUAL:
            lexer_set_token_operator(lexer, OP_BOOL_GREATER_THAN_OR_EQUAL);
            return lexer_expect_space(lexer, status);
        case SYMBOL_GREATER_THAN:
            lexer_set_token_operator(lexer, OP_BOOL_GREATER_THAN);
            return lexer_expect_space(lexer, status);
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
        switch (lexer->tokenizer.token.as.symbol) {
            case SYMBOL_OPAREN:
                lexer_set_token_function(lexer, &identifier);
                return lexer_push_state(lexer, LEXER_STATE_FUNCTION, status) &&
                       lexer_expect_not_space_and_not_code_end(lexer, status);
            case SYMBOL_OBRACKET:
                lexer_set_token_index(lexer, &identifier);
                return lexer_push_state(lexer, LEXER_STATE_INDEX, status) &&
                       lexer_expect_not_space_and_not_code_end(lexer, status);
            case SYMBOL_CPAREN:
                switch (lexer_get_state(lexer)) {
                    case LEXER_STATE_FUNCTION:
                    case LEXER_STATE_PAREN:
                        lexer->already_loaded_next = true;
                        break;
                    default:
                        return unexpected_cparen(status);
                }
                break;
            default:
                return unexpected_token(status);
        }
    }

    lexer_set_token_lookup(lexer, &identifier);

    switch (lexer->tokenizer.token.type) {
        case TOKEN_CODE_END:
        case TOKEN_SPACE:
            return status_ok(status);
        case TOKEN_SYMBOL:
            switch (lexer->tokenizer.token.as.symbol) {
                case SYMBOL_COMMA:
                case SYMBOL_CPAREN:
                case SYMBOL_CBRACKET:
                    return status_ok(status);
                default:
                    break;
            }
        default:
            return expected_space_or_expression_end(status);
    }

    return status_ok(status);
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
    do {
        if (!lexer->already_loaded_next) {
            if (!tokenizer_load_next(&lexer->tokenizer, status)) {
                return false;
            }
        }

        lexer->already_loaded_next = false;
    } while (lexer->tokenizer.token.type == TOKEN_CODE_START);

    switch (lexer->tokenizer.token.type) {
        case TOKEN_UNKNOWN:
            return unknown_token(status);
        case TOKEN_TEXT:
            lexer_handle_text_token(lexer);
            return status_ok(status);
        case TOKEN_NUMBER:
            return lexer_handle_number_token(lexer, status);
        case TOKEN_STRING:
            return lexer_handle_string_token(lexer, status);
        case TOKEN_SYMBOL:
            return lexer_handle_symbol_token(lexer, status);
        case TOKEN_KEYWORD:
            return lexer_handle_keyword_token(lexer, status);
        case TOKEN_IDENTIFIER:
            return lexer_handle_identifier_token(lexer, status);
        case TOKEN_SPACE:
            return unexpected_space(status);
        case TOKEN_CODE_START:
            return status_ok(status);
        default:
            break;
    }

    return unexpected_token(status);
}

/* vi: set et ts=4 sw=4: */
