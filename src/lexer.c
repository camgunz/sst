#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "array.h"
#include "parray.h"
#include "lexer.h"
#include "utils.h"

const rune MathOpValues[MATHOP_MAX] = {
    '+', '-', '*', '/', '%', '^',
};

const rune SymbolValues[SYMBOL_MAX] = {
    '(', ')', '[', ']', '{', '}', ',', '.', '\'', '`', '"', '|'
};

const rune WhitespaceValues[WHITESPACE_MAX] = {
    ' ', '\t', '\r', '\n',
};

const char *BoolOpValues[BOOLOP_MAX] = {
    "==", "!=", ">", ">=", "<", "<=", "&&", "||"
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include", "if", "elif", "else", "endif", "for", "in", "endfor", "raw",
    "endraw", "range"
};

static SSliceStatus seek_to_next_code_tag_start(SSlice *s) {
    SSlice cursor;
    SSliceStatus res;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        if (sslice_starts_with(&cursor, "{{")) {
            break;
        }

        res = sslice_advance_rune(&cursor);

        if (res != SSLICE_OK) {
            return res;
        }
    }

    sslice_shallow_copy(s, &cursor);

    return SSLICE_OK;
}

static bool find_next_code_tag(SSlice *data, SSlice *tag) {
    SSlice tag2;
    char *open;
    SSliceStatus res;

    sslice_shallow_copy(&tag2, data);

    res = seek_to_next_code_tag_start(&tag2);

    if (res != SSLICE_OK) {
        return false;
    }

    open = tag2.data;

    while (true) {
        SSliceStatus res;
        rune in_string = '\0';
        rune r;

        res = sslice_pop_rune(&tag2, &r);

        if (res != SSLICE_OK) {
            return false;
        }

        switch (r) {
            case '\'': {
                switch (in_string) {
                    case '\0': {
                        in_string = '\'';
                        break;
                    }
                    case '\'': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            case '`': {
                switch (in_string) {
                    case '\0': {
                        in_string = '`';
                        break;
                    }
                    case '`': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            case '"': {
                switch (in_string) {
                    case '\0': {
                        in_string = '"';
                        break;
                    }
                    case '"': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }

        if ((in_string == '\0') && sslice_starts_with(&tag2, "}}")) {
            if (sslice_advance_rune(&tag2) != SSLICE_OK) {
                return false;
            }

            if (sslice_advance_rune(&tag2) != SSLICE_OK) {
                return false;
            }

            tag2.len = tag->data - open;
            tag2.data = open;

            sslice_shallow_copy(tag, &tag2);

            return true;
        }
    }
}

LexerStatus token_clear(Token *token) {
    switch (token->type) {
        case TOKEN_TEXT:
            sslice_clear(&token->as.text);
            break;
        case TOKEN_NUMBER:
            mpd_del(token->as.number);
            break;
        case TOKEN_IDENTIFIER:
            sslice_clear(&token->as.identifier);
            break;
        case TOKEN_STRING:
            sslice_clear(&token->as.string);
            break;
        case TOKEN_UNKNOWN:
            break;
        default:
            return LEXER_UNKNOWN_TOKEN;
    }

    token->type = TOKEN_UNKNOWN;
    return LEXER_OK;
}

LexerStatus token_copy(Token *dst, Token *src) {
    token_clear(dst);

    dst->type = src->type;

    switch (src->type) {
        case TOKEN_TEXT:
            sslice_shallow_copy(&dst->as.text, &src->as.text);
            break;
        case TOKEN_NUMBER:
            dst->as.number = src->as.number;
            break;
        case TOKEN_KEYWORD:
            dst->as.keyword = src->as.keyword;
            break;
        case TOKEN_IDENTIFIER:
            sslice_shallow_copy(&dst->as.identifier, &src->as.identifier);
            break;
        case TOKEN_STRING:
            sslice_shallow_copy(&dst->as.string, &src->as.string);
            break;
        case TOKEN_BOOLOP:
            dst->as.bool_op = src->as.bool_op;
            break;
        case TOKEN_UNARY_BOOLOP:
            dst->as.unary_bool_op = src->as.unary_bool_op;
            break;
        case TOKEN_MATHOP:
            dst->as.math_op = src->as.math_op;
            break;
        case TOKEN_SYMBOL:
            dst->as.symbol = src->as.symbol;
            break;
        case TOKEN_WHITESPACE:
            dst->as.whitespace = src->as.whitespace;
            break;
        case TOKEN_UNKNOWN:
            break;
        default:
            return LEXER_UNKNOWN_TOKEN;
            break;
    }

    return LEXER_OK;
}

void token_queue_clear(TokenQueue *token_queue) {
    token_queue->head = 0;
    token_queue->tail = 0;

    for (int i = 0; i < TOKEN_QUEUE_SIZE; i++) {
        token_clear(&token_queue->tokens[i]);
    }
}

uint8_t token_queue_count(TokenQueue *token_queue) {
    uint16_t head = token_queue->head;
    uint16_t tail = token_queue->tail;

    if (head > tail) {
        tail += TOKEN_QUEUE_SIZE;
    }

    return (tail - head) + 1;
}

bool token_queue_empty(TokenQueue *token_queue) {
    return token_queue_count(token_queue) == 0;
}

bool token_queue_full(TokenQueue *token_queue) {
    return token_queue_count(token_queue) == TOKEN_QUEUE_SIZE;
}

bool token_queue_push(TokenQueue *token_queue, Token *token) {
    if (token_queue_full(token_queue)) {
        return false;
    }

    token_queue->tail = (token_queue->tail + 1) % TOKEN_QUEUE_SIZE;

    if (token_queue_full(token_queue)) {
        token_queue->head = (token_queue->head + 1) % TOKEN_QUEUE_SIZE;
    }

    token_copy(&token_queue->tokens[token_queue->tail], token);

    return true;
}

Token* token_queue_pop(TokenQueue *token_queue) {
    if (!token_queue_empty(token_queue)) {
        return NULL;
    }

    Token *token = &token_queue->tokens[token_queue->head];

    token_queue->head++;

    return token;
}

Token* token_queue_push_new(TokenQueue *token_queue) {
    if (token_queue_full(token_queue)) {
        token_queue->head = (token_queue->head + 1) % TOKEN_QUEUE_SIZE;
    }

    token_queue->tail = (token_queue->tail + 1) % TOKEN_QUEUE_SIZE;

    return &token_queue->tokens[token_queue->tail];
}

void lexer_init(Lexer *lexer) {
    lexer_clear(lexer);
    mpd_maxcontext(&lexer->mpd_ctx);
}

void lexer_clear(Lexer *lexer) {
    sslice_clear(&lexer->data);
    sslice_clear(&lexer->tag);
    lexer->in_raw = false;
    token_queue_clear(&lexer->tokens);
}

void lexer_set_data(Lexer *lexer, SSlice *data) {
    lexer_clear(lexer);

    sslice_shallow_copy(&lexer->data, data);
}

LexerStatus lexer_base_load_next(Lexer *lexer, bool skip_whitespace) {
    rune r;
    SSlice start;
    SSliceStatus sstatus;

    while (true) {
        if (sslice_empty(&lexer->data)) {
            return LEXER_END;
        }

        sstatus = sslice_get_first_rune(&lexer->data, &r);

        if (sstatus != SSLICE_OK) {
            return sstatus;
        }

        bool found_whitespace = false;

        for (Whitespace ws = WHITESPACE_FIRST; ws < WHITESPACE_MAX; ws++) {
            if (r == WhitespaceValues[ws]) {
                Token *token = token_queue_push_new(&lexer->tokens);

                token->type = TOKEN_WHITESPACE;
                token->as.whitespace = ws;

                found_whitespace = true;

                break;
            }

        }

        if (!found_whitespace) {
            break;
        }

        if (!skip_whitespace) {
            return LEXER_OK;
        }

        sstatus = sslice_advance_rune(&lexer->data);
        if (sstatus != SSLICE_OK) {
            return sstatus;
        }

        sstatus = sslice_get_first_rune(&lexer->data, &r);
        if (sstatus != SSLICE_OK) {
            return sstatus;
        }
    }

    sslice_shallow_copy(&start, &lexer->data);

    if (rune_is_digit(r) || r == '-' || r == '.') {
        bool found_at_least_one_digit = false;

        while (true) {
            rune r2;

            sstatus = sslice_get_first_rune(&lexer->data, &r2);

            if (sstatus != SSLICE_OK) {
                if (sstatus == SSLICE_END) {
                    break;
                }

                return sstatus;
            }

            if (rune_is_digit(r2)) {
                found_at_least_one_digit = true;
                sslice_pop_rune(&lexer->data, NULL);
                continue;
            }

            if (r2 == ',') {
                sslice_pop_rune(&lexer->data, NULL);
                continue;
            }

            if (r2 == '.') {
                sslice_pop_rune(&lexer->data, NULL);
                continue;
            }

            break;
        }

        if (found_at_least_one_digit) {
            Token *token;
            uint32_t res = 0;
            char *num = strndup(start.data, (lexer->data.data - start.data) - 1);

            if (!num) {
                return LEXER_DATA_MEMORY_EXHAUSTED;
            }

            token = token_queue_push_new(&lexer->tokens);
            token->type = TOKEN_NUMBER;
            token->as.number = mpd_new(&lexer->mpd_ctx);

            if (!token->as.number) {
                free(num);

                return LEXER_DATA_MEMORY_EXHAUSTED;
            }

            mpd_qset_string(token->as.number, num, &lexer->mpd_ctx, &res);

            free(num);

            if (res != 0) {
                return LEXER_INVALID_NUMBER_FORMAT;
            }

            return LEXER_OK;
        }

        return LEXER_INVALID_NUMBER_FORMAT;
    }

#if 0
    if (rune_is_alpha(r) || r == '_') {
        res = sslice_truncate_at_whitespace(&buf);

        if (res != SSLICE_OK) {
            return res;
        }

        if (!validate_identifier(&buf)) {
            return LEXER_INVALID_IDENTIFIER_FORMAT;
        }

        for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
            if (sslice_equals(&buf, KeywordValues[kw])) {
                lexer->token.type = TOKEN_KEYWORD;
                lexer->token.as.keyword = kw;

                return LEXER_OK;
            }
        }

        lexer->token.type = TOKEN_IDENTIFIER;
        sslice_shallow_copy(&lexer->token.as.identifier, &buf);

        return LEXER_OK;
    }

    if (r == '\'' || r == '`' || r == '"') {
        res = sslice_truncate_at(&buf, r);

        switch (res) {
            case SSLICE_END:
            case SSLICE_MEMORY_EXHAUSTED:
            case SSLICE_NOT_ASSIGNED:
            case SSLICE_INVALID_OPTS: {
                return res;
            }
            default: {
                break;
            }
        }

        res = sslice_truncate_runes(&buf, 1);

        switch (res) {
            case SSLICE_END:
            case SSLICE_MEMORY_EXHAUSTED:
            case SSLICE_NOT_ASSIGNED:
            case SSLICE_INVALID_OPTS: {
                return res;
            }
            default: {
                break;
            }
        }

        lexer->token.type = TOKEN_STRING;
        sslice_shallow_copy(&lexer->token.as.string, &buf);

        return sslice_advance_bytes(&lexer->data, buf.len + 1);
    }

    if (uc == '=') {
        if (sslice_pop_char_if_equals(&lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNKNOWN;
            lexer->token.as.literal = uc;
        }
        return LEXER_OK;
    }

    if (uc == '<') {
        if (sslice_pop_char_if_equals(&lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_LESS_THAN_OR_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNARY_BOOLOP;
            lexer->token.as.unary_bool_op = BOOLOP_LESS_THAN;
        }
        return LEXER_OK;
    }

    if (uc == '>') {
        if (sslice_pop_char_if_equals(&lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_GREATER_THAN_OR_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNARY_BOOLOP;
            lexer->token.as.unary_bool_op = BOOLOP_GREATER_THAN;
        }
        return LEXER_OK;
    }

    if (uc == '&') {
        if (sslice_pop_char_if_equals(&lexer->code, '&')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_AND;
        }
        else {
            lexer->token.type = TOKEN_UNKNOWN;
            lexer->token.as.literal = uc;
        }
        return LEXER_OK;
    }

    if (uc == '|') {
        if (sslice_pop_char_if_equals(&lexer->code, '|')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_OR;
        }
        else {
            lexer->token.type = TOKEN_UNKNOWN;
            lexer->token.as.literal = uc;
        }
        return LEXER_OK;
    }

    if (uc == '!') {
        if (sslice_pop_char_if_equals(&lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_NOT_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNARY_BOOLOP;
            lexer->token.as.unary_bool_op = UBOOLOP_NOT;
        }
        return LEXER_OK;
    }

    for (MathOp m = MATHOP_FIRST; m < MATHOP_MAX; m++) {
        if (uc == MathOpValues[m]) {
            lexer->token.type = TOKEN_MATHOP;
            lexer->token.as.math_op = m;
            return LEXER_OK;
        }
    }

    for (Symbol s = SYMBOL_FIRST; s < SYMBOL_MAX; s++) {
        if (uc == SymbolValues[s]) {
            lexer->token.type = TOKEN_SYMBOL;
            lexer->token.as.symbol = s;
            return LEXER_OK;
        }
    }
#endif

    return LEXER_UNKNOWN_TOKEN;
}

Token* lexer_get_current_token(Lexer *lexer) {
    TokenQueue *token_queue = &lexer->tokens;

    if (token_queue_empty(token_queue)) {
        return NULL;
    }

    return &token_queue->tokens[token_queue->tail];
}

/* vi: set et ts=4 sw=4: */

