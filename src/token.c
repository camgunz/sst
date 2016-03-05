#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "token.h"
#include "utils.h"

const char *TokenTypes[TOKEN_MAX] = {
    "Unknown",
    "Text",
    "Number",
    "Keyword",
    "Identifier",
    "String",
    "BoolOp",
    "Unary BoolOp",
    "MathOp",
    "Symbol",
    "Whitespace"
};

bool token_valid(Token *token) {
    switch (token->type) {
        case TOKEN_TEXT:
        case TOKEN_NUMBER:
        case TOKEN_KEYWORD:
        case TOKEN_IDENTIFIER:
        case TOKEN_STRING:
        case TOKEN_BOOLOP:
        case TOKEN_UNARY_BOOLOP:
        case TOKEN_MATHOP:
        case TOKEN_SYMBOL:
        case TOKEN_WHITESPACE:
            return true;
        case TOKEN_UNKNOWN:
        default:
            return false;
    }
}

TokenStatus token_clear(Token *token) {
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
            return TOKEN_UNKNOWN_TOKEN;
    }

    token->type = TOKEN_UNKNOWN;

    return TOKEN_OK;
}

TokenStatus token_copy(Token *dst, Token *src) {
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
            return TOKEN_UNKNOWN_TOKEN;
    }

    return TOKEN_OK;
}

char* token_to_string(Token *token) {
    switch (token->type) {
        case TOKEN_TEXT:
            return sslice_to_c_string(&token->as.text);
        case TOKEN_NUMBER:
            return mpd_to_sci(token->as.number, 0);
        case TOKEN_KEYWORD:
            return strdup(KeywordValues[token->as.keyword]);
        case TOKEN_IDENTIFIER:
            return sslice_to_c_string(&token->as.identifier);
        case TOKEN_STRING:
            return sslice_to_c_string(&token->as.string);
        case TOKEN_BOOLOP:
            return strdup(BoolOpValues[token->as.bool_op]);
        case TOKEN_UNARY_BOOLOP:
            return chardup('!');
        case TOKEN_MATHOP:
            return chardup(MathOpValues[token->as.math_op]);
        case TOKEN_SYMBOL:
            return chardup(SymbolValues[token->as.symbol]);
        case TOKEN_WHITESPACE:
            switch (token->as.whitespace) {
                case WHITESPACE_SPACE:
                    return strdup("<space>");
                case WHITESPACE_TAB:
                    return strdup("<tab>");
                case WHITESPACE_CARRIAGE_RETURN:
                    return strdup("<cr>");
                case WHITESPACE_NEWLINE:
                    return strdup("<nl>");
                case WHITESPACE_MAX:
                default:
                    return strdup("<ws_unknown>");
            }
            break;
        case TOKEN_UNKNOWN:
        case TOKEN_MAX:
        default:
            return strdup("Unknown");
    }
}

void token_queue_clear(TokenQueue *token_queue) {
    token_queue->head = 0;
    token_queue->tail = 0;

    for (int i = 0; i < TOKEN_QUEUE_SIZE; i++) {
        token_queue->tokens[i].type = TOKEN_UNKNOWN;
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

    if (!token_valid(token)) {
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

/* vi: set et ts=4 sw=4: */

