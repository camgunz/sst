#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"

#define invalid_syntax(status) status_failure( \
    status,                                    \
    "tokenizer",                               \
    TOKENIZER_INVALID_SYNTAX,                  \
    "Invalid syntax"                           \
)

#define eof(status) status_failure( \
    status,                         \
    "tokenizer",                    \
    TOKENIZER_EOF,                  \
    "EOF"                           \
)

#define unexpected_eof(status) status_failure( \
    status,                                    \
    "tokenizer",                               \
    TOKENIZER_UNEXPECTED_EOF,                  \
    "Unexpected EOF"                           \
)

#define invalid_number_format(status) status_failure( \
    status,                                           \
    "tokenizer",                                      \
    TOKENIZER_INVALID_NUMBER_FORMAT,                  \
    "Invalid number format"                           \
)

#define hanging_raw(status) status_failure( \
    status,                                 \
    "tokenizer",                            \
    TOKENIZER_HANGING_RAW,                  \
    "Raw tag not closed"                    \
)

#define unknown_token(status) status_failure( \
    status,                                   \
    "tokenizer",                              \
    TOKENIZER_UNKNOWN_TOKEN,                  \
    "Unknown token"                           \
)

#define not_handled(status) status_failure( \
    status,                                 \
    "tokenizer",                            \
    TOKENIZER_TOKEN_NOT_HANDLED,            \
    "Token not handled"                     \
)

#define invalid_whitespace(status) status_failure( \
    status,                                        \
    "tokenizer",                                   \
    TOKENIZER_INVALID_WHITESPACE,                  \
    "Invalid whitespace"                           \
)

static inline
bool tokenizer_skip_rune(Tokenizer *tokenizer, Status *status) {
    if (sslice_skip_rune(tokenizer->data, status)) {
        tokenizer->column++;
        return status_ok(status);
    }

    return false;
}

static inline
bool tokenizer_skip_runes(Tokenizer *tokenizer, size_t len, Status *status) {
    if (sslice_skip_runes(tokenizer->data, len, status)) {
        tokenizer->column += len;
        return status_ok(status);
    }

    return false;
}

static inline
bool tokenizer_skip_rune_if_equals(Tokenizer *tokenizer, rune r,
                                                         Status *status) {
    if (sslice_skip_rune_if_equals(tokenizer->data, r, status)) {
        size_t len = tokenizer->data->len;

        tokenizer->column += len - tokenizer->data->len;

        return status_ok(status);
    }

    return false;
}

static inline
bool tokenizer_seek_past_subslice(Tokenizer *tokenizer, SSlice *subslice,
                                                        Status *status) {

    size_t saved_len = tokenizer->data->len;

    if (sslice_seek_past_subslice(tokenizer->data, subslice, status)) {
        tokenizer->column += (saved_len - tokenizer->data->len);

        return status_ok(status);
    }

    return false;
}

static inline
void tokenizer_set_token_text(Tokenizer *tokenizer, SSlice *text) {
    tokenizer->token.type = TOKEN_TEXT;
    tokenizer->token.location = text->data;
    sslice_copy(&tokenizer->token.as.text, text);
}

static inline
void tokenizer_set_token_number(Tokenizer *tokenizer, SSlice *number) {
    tokenizer->token.type = TOKEN_NUMBER;
    tokenizer->token.location = number->data;
    sslice_copy(&tokenizer->token.as.number, number);
}

static inline
void tokenizer_set_token_string(Tokenizer *tokenizer, SSlice *string) {
    tokenizer->token.type = TOKEN_STRING;
    tokenizer->token.location = string->data;
    sslice_copy(&tokenizer->token.as.string, string);
}

static inline
void tokenizer_set_token_symbol(Tokenizer *tokenizer, Symbol symbol) {
    tokenizer->token.type = TOKEN_SYMBOL;
    tokenizer->token.location = tokenizer->data->data;
    tokenizer->token.as.symbol = symbol;
}

static inline
void tokenizer_set_token_keyword(Tokenizer *tokenizer, Keyword keyword) {
    tokenizer->token.type = TOKEN_KEYWORD;
    tokenizer->token.location = tokenizer->data->data;
    tokenizer->token.as.keyword = keyword;
}

static inline
void tokenizer_set_token_identifier(Tokenizer *tokenizer, SSlice *identifier) {
    tokenizer->token.type = TOKEN_IDENTIFIER;
    tokenizer->token.location = identifier->data;
    sslice_copy(&tokenizer->token.as.identifier, identifier);
}

static inline
void tokenizer_set_token_space(Tokenizer *tokenizer) {
    tokenizer->token.type = TOKEN_SPACE;
    tokenizer->token.location = tokenizer->data->data;
}

static inline
bool tokenizer_set_token_code_start(Tokenizer *tokenizer, Status *status) {
    tokenizer->token.type = TOKEN_CODE_START;
    tokenizer->token.location = tokenizer->data->data;

    return tokenizer_skip_runes(tokenizer, 3, status);
}

static inline
bool tokenizer_set_token_code_end(Tokenizer *tokenizer, Status *status) {
    tokenizer->token.type = TOKEN_CODE_END;
    tokenizer->token.location = tokenizer->data->data;

    return tokenizer_skip_runes(tokenizer, 3, status);
}

static
bool tokenizer_seek_to_next_code_tag_start(Tokenizer *tokenizer,
                                           Status *status) {
    SSlice cursor;
    size_t lines_skipped = 0;
    size_t columns_skipped = 0;

    sslice_copy(&cursor, tokenizer->data);

    while (cursor.byte_len >= 2) {
        rune r;
        SSlice start;

        sslice_copy(&start, &cursor);

        if (!sslice_pop_rune(&cursor, &r, status)) {
            return false;
        }

        switch (r) {
            case '\n':
                lines_skipped++;
                columns_skipped = 1;
                continue;
            case '{':
                break;
            default:
                columns_skipped++;
                continue;
        }

        if (!sslice_pop_rune(&cursor, &r, status)) {
            return false;
        }

        switch (r) {
            case '\n':
                lines_skipped++;
                columns_skipped = 1;
                continue;
            case '{':
                break;
            default:
                columns_skipped += 2;
                continue;
        }

        if (!sslice_pop_rune(&cursor, &r, status)) {
            return false;
        }

        if (r != ' ') {
            return invalid_syntax(status);
        }

        sslice_copy(tokenizer->data, &start);
        tokenizer->line += lines_skipped;
        tokenizer->column = columns_skipped + 1;

        return status_ok(status);
    }

    return not_found(status);
}

static
bool tokenizer_handle_raw(Tokenizer *tokenizer, Status *status) {
    SSlice raw_text;
    SSlice cursor;

    /* {{ raw }} is 9 runes */
    if (!tokenizer_skip_runes(tokenizer, 9, status)) {
        return false;
    }

    sslice_copy(&raw_text, tokenizer->data);
    sslice_copy(&cursor, tokenizer->data);

    if (!sslice_seek_to_cstr(&cursor, "{{ endraw }}", status)) {
        if (status_match(status, "base", ERROR_NOT_FOUND)) {
            return hanging_raw(status);
        }

        return false;
    }

    if (!sslice_truncate_at_subslice(&raw_text, &cursor, status)) {
        return false;
    }

    tokenizer_set_token_text(tokenizer, &raw_text);

    if (!tokenizer_seek_past_subslice(tokenizer, &raw_text, status)) {
        return false;
    }

    /* {{ endraw }} is 12 runes */
    return tokenizer_skip_runes(tokenizer, 12, status);
}

static
bool tokenizer_handle_number(Tokenizer *tokenizer, Status *status) {
    SSlice cursor;
    SSlice number;
    rune r;
    bool found_at_least_one_digit = false;

    sslice_copy(&number, tokenizer->data);
    sslice_copy(&cursor, tokenizer->data);

    while (sslice_get_first_rune(&cursor, &r, status)) {
        bool is_digit = rune_is_digit(r);

        if (is_digit) {
            found_at_least_one_digit = true;
        }

        if (!(is_digit || (r == '_') || (r == '.'))) {
            break;
        }

        if (!sslice_skip_rune(&cursor, status)) {
            return false;
        }
    }

    if (!status_is_ok(status)) {
        if (status_match(status, "sslice", SSLICE_EMPTY)) {
            return unexpected_eof(status);
        }

        return false;
    }

    if (!found_at_least_one_digit) {
        return invalid_number_format(status);
    }

    if (!sslice_truncate_at_subslice(&number, &cursor, status)) {
        return false;
    }

    tokenizer_set_token_number(tokenizer, &number);

    return tokenizer_seek_past_subslice(tokenizer, &number, status);
}

static
bool tokenizer_handle_string(Tokenizer *tokenizer, rune r, Status *status) {
    SSlice string;
    SSlice cursor;

    sslice_copy(&string, tokenizer->data);

    if (!sslice_skip_rune(&string, status)) {
        return false;
    }

    sslice_copy(&cursor, &string);

    if (!sslice_seek_to(&cursor, r, status)) {
        if (status_match(status, "base", ERROR_NOT_FOUND)) {
            return unexpected_eof(status);
        }

        return false;
    }

    if (!sslice_truncate_at_subslice(&string, &cursor, status)) {
        return false;
    }

    tokenizer_set_token_string(tokenizer, &string);

    return tokenizer_skip_runes(tokenizer, string.len + 2, status);
}

static
bool tokenizer_handle_symbol(Tokenizer *tokenizer, rune r, Status *status) {
    switch (r) {
        case '(':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_OPAREN);
            return tokenizer_skip_rune(tokenizer, status);
        case ')':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_CPAREN);
            return tokenizer_skip_rune(tokenizer, status);
        case '+':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_PLUS);
            return tokenizer_skip_rune(tokenizer, status);
        case '-':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_MINUS);
            return tokenizer_skip_rune(tokenizer, status);
        case '*':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_ASTERISK);
            return tokenizer_skip_rune(tokenizer, status);
        case '/':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_FORWARD_SLASH);
            return tokenizer_skip_rune(tokenizer, status);
        case '%':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_PERCENT);
            return tokenizer_skip_rune(tokenizer, status);
        case '^':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_CARET);
            return tokenizer_skip_rune(tokenizer, status);
        case ',':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_COMMA);
            return tokenizer_skip_rune(tokenizer, status);
        case '[':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_OBRACKET);
            return tokenizer_skip_rune(tokenizer, status);
        case ']':
            tokenizer_set_token_symbol(tokenizer, SYMBOL_CBRACKET);
            return tokenizer_skip_rune(tokenizer, status);
        default:
            break;
    }

    rune next_r;
    SSlice cursor;

    sslice_copy(&cursor, tokenizer->data);

    if (!sslice_skip_rune(&cursor, status)) {
        return false;
    }

    if (!sslice_get_first_rune(&cursor, &next_r, status)) {
        return false;
    }

    if (r == '&') {
        if (next_r != '&') {
            return invalid_syntax(status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_AND);
        return tokenizer_skip_runes(tokenizer, 2, status);
    }

    if (r == '|') {
        if (next_r != '|') {
            return invalid_syntax(status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_OR);
        return tokenizer_skip_runes(tokenizer, 2, status);
    }

    if (r == '=') {
        if (next_r != '=') {
            return invalid_syntax(status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_EQUAL);
        return tokenizer_skip_runes(tokenizer, 2, status);
    }

    if (r == '!') {
        if (next_r == '=') {
            tokenizer_set_token_symbol(tokenizer, SYMBOL_NOT_EQUAL);
            return tokenizer_skip_runes(tokenizer, 2, status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_EXCLAMATION_POINT);
        return status_ok(status);
    }

    if (r == '<') {
        if (next_r == '=') {
            tokenizer_set_token_symbol(tokenizer, SYMBOL_LESS_THAN_OR_EQUAL);
            return tokenizer_skip_runes(tokenizer, 2, status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_LESS_THAN);
        return status_ok(status);
    }

    if (r == '>') {
        if (next_r == '=') {
            tokenizer_set_token_symbol(
                tokenizer,
                SYMBOL_GREATER_THAN_OR_EQUAL
            );
            return tokenizer_skip_runes(tokenizer, 2, status);
        }

        tokenizer_set_token_symbol(tokenizer, SYMBOL_GREATER_THAN);
        return status_ok(status);
    }

    return not_handled(status);
}

static
bool tokenizer_handle_word(Tokenizer *tokenizer, Status *status) {
    for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
        if (sslice_starts_with_cstr(tokenizer->data, KeywordValues[kw])) {
            SSlice cursor;
            size_t keyword_length = strlen(KeywordValues[kw]);

            sslice_copy(&cursor, tokenizer->data);

            if (!sslice_skip_runes(&cursor, keyword_length, status)) {
                return false;
            }

            if (!sslice_first_rune_equals(&cursor, ' ', status)) {
                if (status_match(status, "base", ERROR_NOT_EQUAL)) {
                    continue;
                }

                return false;
            }

            tokenizer_set_token_keyword(tokenizer, kw);

            return tokenizer_skip_runes(tokenizer, keyword_length, status);
        }
    }

    rune r;
    SSlice cursor;
    SSlice identifier;

    sslice_copy(&cursor, tokenizer->data);
    sslice_copy(&identifier, tokenizer->data);

    while (sslice_get_first_rune(&cursor, &r, status)) {
        if (rune_is_alpha(r) || rune_is_digit(r) || r == '_' || r == '.') {
            if (!sslice_skip_rune(&cursor, status)) {
                return false;
            }
            continue;
        }

        if (!sslice_truncate_at_subslice(&identifier, &cursor, status)) {
            return false;
        }

        tokenizer_set_token_identifier(tokenizer, &identifier);

        return tokenizer_seek_past_subslice(tokenizer, &identifier, status);
    }

    return not_handled(status);
}

static
bool tokenizer_handle_whitespace(Tokenizer *tokenizer, rune r,
                                                       Status *status) {
    switch (r) {
        case ' ':
            tokenizer_set_token_space(tokenizer);
            return tokenizer_skip_rune(tokenizer, status);
        default:
            return invalid_whitespace(status);
    }
}

static
bool tokenizer_load_next_code_token(Tokenizer *tokenizer, Status *status) {
    rune r;

    if (!sslice_get_first_rune(tokenizer->data, &r, status)) {
        if (status_match(status, "sslice", SSLICE_EMPTY)) {
            return unexpected_eof(status);
        }

        return false;
    }

    if (rune_is_whitespace(r)) {
        return tokenizer_handle_whitespace(tokenizer, r, status);
    }

    if (r == '\'' || r == '"' || r == '`') {
        return tokenizer_handle_string(tokenizer, r, status);
    }

    if (rune_is_digit(r) || r == '.') {
        return tokenizer_handle_number(tokenizer, status);
    }

    if (rune_is_alpha(r) || r == '_') {
        return tokenizer_handle_word(tokenizer, status);
    }

    if (tokenizer_handle_symbol(tokenizer, r, status)) {
        return status_ok(status);
    }

    if (status_match(status, "tokenizer", TOKENIZER_TOKEN_NOT_HANDLED)) {
        return false;
    }

    return unknown_token(status);
}

void tokenizer_init(Tokenizer *tokenizer, SSlice *data) {
    tokenizer_clear(tokenizer);
    tokenizer->data = data;
}

void tokenizer_clear(Tokenizer *tokenizer) {
    tokenizer->data = NULL;
    tokenizer->line = 1;
    tokenizer->column = 1;
    tokenizer->token.type = TOKEN_UNKNOWN;
    tokenizer->token.location = NULL;
    tokenizer->in_code = false;
}

bool tokenizer_load_next(Tokenizer *tokenizer, Status *status) {
    SSlice start;

    if (tokenizer->in_code) {
        if (sslice_starts_with_cstr(tokenizer->data, "}}")) {
            return invalid_syntax(status);
        }

        if (sslice_starts_with_cstr(tokenizer->data, " }}")) {
            tokenizer->in_code = false;
            return tokenizer_set_token_code_end(tokenizer, status);
        }

        if (!tokenizer_load_next_code_token(tokenizer, status)) {
            if (status_match(status, "base", ERROR_NOT_FOUND)) {
                return eof(status);
            }

            return false;
        }

        return status_ok(status);
    }

    if (sslice_empty(tokenizer->data)) {
        return eof(status);
    }

    sslice_copy(&start, tokenizer->data);

    if (tokenizer_seek_to_next_code_tag_start(tokenizer, status)) {
        if (tokenizer->data->data != start.data) {
            if (!sslice_truncate_at_subslice(&start, tokenizer->data, status)) {
                return false;
            }

            tokenizer_set_token_text(tokenizer, &start);

            return status_ok(status);
        }

        if (sslice_starts_with_cstr(tokenizer->data, "{{ raw }}")) {
            return tokenizer_handle_raw(tokenizer, status);
        }

        tokenizer->in_code = true;
        return tokenizer_set_token_code_start(tokenizer, status);
    }

    if (!status_match(status, "base", ERROR_NOT_FOUND)) {
        return false;
    }

    status_init(status);

    tokenizer_set_token_text(tokenizer, tokenizer->data);

    sslice_seek_to_end(tokenizer->data);

    return status_ok(status);
}

/* vi: set et ts=4 sw=4: */
