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
    return sslice_skip_rune(tokenizer->data, status);
}

static inline
bool tokenizer_skip_runes(Tokenizer *tokenizer, size_t len, Status *status) {
    return sslice_skip_runes(tokenizer->data, len, status);
}

static inline
bool tokenizer_skip_rune_if_equals(Tokenizer *tokenizer, rune r,
                                                         Status *status) {
    size_t len = tokenizer->data->len;
    bool res = sslice_skip_rune_if_equals(tokenizer->data, r, status);

    if (res) {
        tokenizer->column += len - tokenizer->data->len;
    }

    return res;
}

static inline
bool tokenizer_seek_past_subslice(Tokenizer *tokenizer, SSlice *subslice,
                                                        Status *status) {
    return sslice_seek_past_subslice(tokenizer->data, subslice, status);
}

static inline
void tokenizer_set_token_text(Tokenizer *tokenizer, SSlice *text) {
    tokenizer->token.type = TOKEN_TEXT;
    tokenizer->token.location = tokenizer->data->data;
    sslice_copy(&tokenizer->token.as.text, text);
}

static inline
void tokenizer_set_token_number(Tokenizer *tokenizer, SSlice *number) {
    tokenizer->token.type = TOKEN_NUMBER;
    tokenizer->token.location = tokenizer->data->data;
    sslice_copy(&tokenizer->token.as.number, number);
}

static inline
void tokenizer_set_token_string(Tokenizer *tokenizer, SSlice *string) {
    tokenizer->token.type = TOKEN_STRING;
    tokenizer->token.location = tokenizer->data->data;
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
    tokenizer->token.location = tokenizer->data->data;
    sslice_copy(&tokenizer->token.as.identifier, identifier);
}

static inline
void tokenizer_set_token_whitespace(Tokenizer *tokenizer, Whitespace ws) {
    tokenizer->token.type = TOKEN_WHITESPACE;
    tokenizer->token.location = tokenizer->data->data;
    tokenizer->token.as.whitespace = ws;
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
        size_t rune_size;

        if (!utf8_get_first_rune_len(cursor.data, &r, &rune_size, status)) {
            return false;
        }

        cursor.len--;
        cursor.byte_len -= rune_size;
        cursor.data += rune_size;

        if (r == '\n') {
            lines_skipped++;
            columns_skipped = 0;
            continue;
        }

        if (r != '{') {
            columns_skipped++;
            continue;
        }

        if (!utf8_get_first_rune_len(cursor.data, &r, &rune_size, status)) {
            return false;
        }

        cursor.len--;
        cursor.byte_len -= rune_size;
        cursor.data += rune_size;

        columns_skipped++;

        if (r != '{') {
            continue;
        }

        cursor.len--;
        cursor.byte_len -= rune_size;
        cursor.data += rune_size;

        sslice_copy(tokenizer->data, &cursor);
        tokenizer->line += lines_skipped;
        tokenizer->column = columns_skipped;

        if (!tokenizer_skip_rune_if_equals(tokenizer, ' ', status)) {
            if (status_match(status, "sslice", SSLICE_NOT_EQUAL)) {
                return invalid_syntax(status);
            }

            return false;
        }

        tokenizer->data->len--;
        tokenizer->data->byte_len--;
        tokenizer->data->data++;

        return status_ok(status);
    }

    return not_found(status);
}

static
bool tokenizer_handle_number(Tokenizer *tokenizer, Status *status) {
    SSlice cursor;
    SSlice number;
    bool found_at_least_one_digit = false;

    sslice_copy(&number, tokenizer->data);
    sslice_copy(&cursor, tokenizer->data);

    do {
        rune r;
        bool is_digit;

        if (!sslice_get_first_rune(&cursor, &r, status)) {
            if (status_match(status, "sslice", SSLICE_EMPTY)) {
                return unexpected_eof(status);
            }

            return false;
        }

        is_digit = rune_is_digit(r);

        if (is_digit) {
            found_at_least_one_digit = true;
        }

        if (!(is_digit || (r == ',') || (r == '.'))) {
            break;
        }
    } while (sslice_skip_rune(&cursor, status));

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

    sslice_copy(&string, tokenizer->data);

    if (!sslice_skip_rune(&string, status)) {
        return false;
    }

    if (!sslice_truncate_at(&string, r, status)) {
        return false;
    }

    tokenizer_set_token_string(tokenizer, &string);

    return tokenizer_skip_runes(tokenizer, string.len + 1, status);
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
    SSlice cursor;
    rune r;

    sslice_copy(&cursor, tokenizer->data);

    if (!sslice_truncate_at_whitespace(&cursor, status)) {
        return false;
    }

    for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
        if (sslice_equals_cstr(&cursor, KeywordValues[kw])) {
            tokenizer_set_token_keyword(tokenizer, kw);

            return tokenizer_seek_past_subslice(tokenizer, &cursor, status);
        }
    }

    SSlice identifier;
    sslice_copy(&identifier, tokenizer->data);

    while (sslice_pop_rune(&cursor, &r, status)) {
        if (!(rune_is_alpha(r) || rune_is_digit(r) || r == '_')) {
            if (!sslice_truncate_at_subslice(&identifier, &cursor, status)) {
                return false;
            }

            tokenizer_set_token_identifier(tokenizer, &identifier);

            return tokenizer_seek_past_subslice(tokenizer, &cursor, status);
        }
    }

    if (status_match(status, "sslice", SSLICE_EMPTY)) {
        return unexpected_eof(status);
    }

    return false;
}

static
bool tokenizer_handle_whitespace(Tokenizer *tokenizer, rune r,
                                                       Status *status) {
    rune next_r;
    SSlice cursor;

    switch (r) {
        case ' ':
            tokenizer_set_token_whitespace(tokenizer, WHITESPACE_SPACE);
            break;
        case '\t':
            tokenizer_set_token_whitespace(tokenizer, WHITESPACE_TAB);
            break;
        case '\n':
            tokenizer_set_token_whitespace(tokenizer, WHITESPACE_NEWLINE);
            break;
        case '\r':
            sslice_copy(&cursor, tokenizer->data);
            if (!sslice_skip_rune(&cursor, status)) {
                return false;
            }

            if (!sslice_get_first_rune(&cursor, &next_r, status)) {
                if (status_match(status, "sslice", SSLICE_EMPTY)) {
                    return unexpected_eof(status);
                }

                return false;
            }

            if (next_r != '\n') {
                return invalid_whitespace(status);
            }

            tokenizer_set_token_whitespace(tokenizer, WHITESPACE_NEWLINE);

            if (!tokenizer_skip_rune(tokenizer, status)) {
                return false;
            }
        default:
            return invalid_whitespace(status);
    }

    return tokenizer_skip_rune(tokenizer, status);
}

static
bool tokenizer_load_next_code_token(Tokenizer *tokenizer, Status *status) {
    rune r;

    if (sslice_starts_with_cstr(tokenizer->data, "{{ raw }}")) {
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

    if (rune_is_digit(r) || r == '+' || r == '-' || r == '.') {
        return tokenizer_handle_number(tokenizer, status);
    }

    if (rune_is_alpha(r) || r == '_') {
        return tokenizer_handle_word(tokenizer, status);
    }

    if (!tokenizer_handle_symbol(tokenizer, r, status)) {
        if (!status_match(status, "tokenizer", TOKENIZER_TOKEN_NOT_HANDLED)) {
            return false;
        }
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
    if (tokenizer->in_code) {
        if (sslice_equals_cstr(tokenizer->data, " }}")) {
            if (!tokenizer_skip_runes(tokenizer, 3, status)) {
                return false;
            }

            tokenizer->in_code = false;
        }
        else if (sslice_equals_cstr(tokenizer->data, "}}")) {
            return invalid_syntax(status);
        }
        else {
            return tokenizer_load_next_code_token(tokenizer, status);
        }
    }

    if (sslice_empty(tokenizer->data)) {
        return eof(status);
    }

    if (tokenizer_seek_to_next_code_tag_start(tokenizer, status)) {
        tokenizer_set_token_text(tokenizer, tokenizer->data);

        if (!tokenizer_skip_runes(tokenizer, 3, status)) {
            return false;
        }

        tokenizer->in_code = true;

        return status_ok(status);
    }

    if (!status_match(status, "base", ERROR_NOT_FOUND)) {
        return false;
    }

    tokenizer_set_token_text(tokenizer, tokenizer->data);

    return status_ok(status);
}

/* vi: set et ts=4 sw=4: */
