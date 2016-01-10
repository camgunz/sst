#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "str.h"
#include "lexer.h"

const gunichar MathOpValues[MATHOP_MAX] = {
    '+', '-', '*', '/', '%', '^',
};

const gunichar SymbolValues[SYMBOL_MAX] = {
    '(', ')', '[', ']', ',', '.', ''', '`', '"'
};

const gunichar WhitespaceValues[WHITESPACE_MAX] = {
    ' ', '\t', '\r', '\n',
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include" "if", "elif", "else", "endif", "for", "in", "endfor", "raw",
    "endraw",
};


void lexer_clear(Lexer *lexer) {
    string_clear(&lexer->code);
    lexer->token.type = TOKEN_UNKNOWN;
}

void lexer_init(Lexer *lexer, String *code) {
    lexer_clear(lexer);

    lexer->code.data = code->data;
    lexer->code.len = code->len;
}

LexerStatus lexer_load_next(Lexer *lexer) {
    String buf;
    gunichar uc;

    if (string_empty(&lexer->code)) {
        return LEXER_EOF;
    }

    buf.data = lexer->code.data;
    buf.len = 0;

    if (!string_pop_char(&lexer->code, &uc)) {
        return LEXER_EOF;
    }

    if (uc == '-' || g_unichar_is_digit(uc)) {
        bool found_at_least_one_digit = false;
        bool found_period = false;

        while (true) {
            gunichar nuc;

            if (!string_first_char(&lexer->code, &nuc)) {
                return LEXER_EOF;
            }

            if (g_unichar_isdigit(nuc)) {
                found_at_least_one_digit = true;
                string_pop_char(&lexer->code, NULL);
                continue;
            }

            if (nuc == ',') {
                string_pop_char(&lexer->code, NULL);
                continue;
            }

            if (nuc == '.') {
                if (!found_period) {
                    found_period = true;
                    string_pop_char(&lexer->code, NULL);
                    continue;
                }
            }

            break;
        }

        if (found_at_least_one_digit) {
            buf.len = lexer->code.data - buf.data;
            lexer->token.type = TOKEN_NUMBER;
            lexer->token.as.number.data = buf.data;
            lexer->token.as.number.len = buf.len;
            lexer->code.len -= buf.len;
            return LEXER_OK
        }
    }

    if (g_unichar_is_alnum(uc)) {
        bool at_least_one_alnum = true;

        while (string_pop_char_if_alnum(&lexer->code, NULL)) {
        }

        buf.len = lexer->code.data - buf.data;

        for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
            if (string_equals(&buf, KeywordValues[kw])) {
                lexer->token.type = TOKEN_KEYWORD;
                lexer->token.as.keyword = kw;
                return LEXER_OK;
            }
        }

        lexer->token.type = TOKEN_IDENTIFIER;
        lexer->token.as.identifier.data = buf.data;
        lexer->token.as.identifier.len = buf.len;
        lexer->code.len -= buf.len;
        return LEXER_OK;
    }

    if (uc == '\'' || uc == '`' || uc == '"') {
        gchar *string_start = buf.data;
        gchar *string_end = string_find(&lexer->code, uc);

        if (!string_end) {
            return LEXER_EOF;
        }

        string_start = g_utf8_next_char(string_start);

        if (!string_start) {
            return LEXER_EOF;
        }

        if (string_start > string_end) {
            return LEXER_INTERNAL_ERROR;
        }

        lexer->token.type = TOKEN_STRING;
        lexer->token.as.string.data = string_start;
        lexer->token.as.string.len = string_end - string_start;

        string_end = g_utf8_next_char(string_end);

        if (!string_end) {
            lexer->code.data = NULL;
            lexer->code.len = 0;
        }
        else {
            lexer->code.data = string_end;
            lexer->code.len -= buf.data - string_end;
        }

        return LEXER_OK;
    }

    if (uc == '=') {
        if (string_pop_char_if_equals(lexer->code, '=')) {
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
        if (string_pop_char_if_equals(lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_LESS_THAN_OR_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNARY_BOOLOP;
            lexer->token.as.unary_bool_op = UBOOLOP_LESS_THAN;
        }
        return LEXER_OK;
    }

    if (uc == '>') {
        if (string_pop_char_if_equals(lexer->code, '=')) {
            lexer->token.type = TOKEN_BOOLOP;
            lexer->token.as.bool_op = BOOLOP_GREATER_THAN_OR_EQUAL;
        }
        else {
            lexer->token.type = TOKEN_UNARY_BOOLOP;
            lexer->token.as.unary_bool_op = UBOOLOP_GREATER_THAN;
        }
        return LEXER_OK;
    }

    if (uc == '&') {
        if (string_pop_char_if_equals(lexer->code, '&')) {
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
        if (string_pop_char_if_equals(lexer->code, '|')) {
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
        if (string_pop_char_if_equals(lexer->code, '=')) {
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

    for (Whitespace ws = WHITESPACE_FIRST; ws < WHITESPACE_SPACE; ws++) {
        if (uc == WhitespaceValues[ws]) {
            lexer->token.type = TOKEN_WHITESPACE;
            lexer->token.as.whitespace = ws;
            return LEXER_OK;
        }
    }

    lexer->token.type = TOKEN_UNKNOWN;
    lexer->token.as.literal = uc
    return LEXER_UNKNOWN_TOKEN;
}

/* vi: set et ts=4 sw=4: */

