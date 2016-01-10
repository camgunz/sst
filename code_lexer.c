#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "str.h"
#include "code_lexer.h"

const gunichar MathOpValues[MATHOP_MAX] = {
    '+',
    '-',
    '*',
    '/',
    '%',
    '^',
};

const gunichar SymbolValues[SYMBOL_MAX] = {
    '(',
    ')',
    '[',
    ']',
    ',',
    '.',
    ''',
    '`',
    '"'
};

const gunichar WhitespaceValues[WHITESPACE_MAX] = {
    ' ',
    '\t',
    '\r',
    '\n',
};

const gunichar BooleanValues[BOOLEAN_MAX] = {
    '!',
    '=',
    '<',
    '>',
    '&',
    '|',
};

const char *UnaryBoolOpValues[UBOOLOP_MAX] = {
    "!"
};

const char *BoolOpValues[BOOLOP_MAX] = {
    "==",
    "!=",
    ">",
    ">=",
    "<",
    "<=",
    "&&",
    "||"
};

const char *BoolOpValues[BOOLOP_MAX] = {
    "==",
    "!=",
    ">",
    ">=",
    "<",
    "<=",
    "&&",
    "||"
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include"
    "if",
    "elif",
    "else",
    "endif",
    "for",
    "in",
    "endfor",
    "raw",
    "endraw",
    "range"
};

void code_lexer_clear(CodeLexer *code_lexer) {
    string_clear(&code_lexer->code);
    code_lexer->token.type = TOKEN_UNKNOWN;
}

void code_lexer_init(CodeLexer *code_lexer, String *code) {
    code_lexer_clear(code_lexer);

    code_lexer->code.data = code->data;
    code_lexer->code.len = code->len;
}

bool code_lexer_load_next(CodeLexer *code_lexer) {
    String buf;
    gunichar uc;

    if (!code_lexer->code.data) {
        return false;
    }

    if (code_lexer->code.len == 0) {
        return false;
    }

    buf.data = code_lexer->code.data;
    buf.len = 0;

    if (!string_pop_char(&code_lexer->code, &uc)) {
        return false;
    }

    if (g_unichar_is_digit(uc)) {
        bool found_at_least_one_digit = false;
        bool found_period = false;

        while (true) {
            gunichar nuc;

            if (!string_first_char(&code_lexer->code, &nuc)) {
                return false;
            }

            if (g_unichar_isdigit(nuc)) {
                found_at_least_one_digit = true;
                string_pop_char(&code_lexer->code, NULL);
                continue;
            }

            if (nuc == ',') {
                string_pop_char(&code_lexer->code, NULL);
                continue;
            u

            if (nuc == '.') {
                if (!found_period) {
                    found_period = true;
                    string_pop_char(&code_lexer->code, NULL);
                    continue;
                }
            }

            break;
        }

        if (found_at_least_one_digit) {
            buf.len = code_lexer->code.data - buf.data;
            code_lexer->token.type = TOKEN_NUMBER;
            code_lexer->token.as.number = parse_number(*buf);
            code_lexer->code.data = buf.data + buf.len;
            code_lexer->code.len -= buf.len;
            return true;
        }
    }

    if (uc == '-') {
        bool found_at_least_one_digit = false;
        bool found_period = false;

        while (true) {
            gunichar nuc;

            if (!string_first_char(&code_lexer->code, &nuc)) {
                return false;
            }

            if (g_unichar_isdigit(nuc)) {
                found_at_least_one_digit = true;
                string_pop_char(&code_lexer->code, NULL);
                continue;
            }

            if (nuc == ',') {
                string_pop_char(&code_lexer->code, NULL);
                continue;
            }

            if (nuc == '.') {
                if (!found_period) {
                    found_period = true;
                    string_pop_char(&code_lexer->code, NULL);
                    continue;
                }
            }

            break;
        }

        if (found_at_least_one_digit) {
            buf.len = code_lexer->code.data - buf.data;
            code_lexer->token.type = TOKEN_NUMBER;
            code_lexer->token.as.number = parse_number(*buf);
            code_lexer->code.data = buf.data + buf.len;
            code_lexer->code.len -= buf.len;
            return true;
        }
    }

    if (g_unichar_is_alnum(uc)) {
        bool at_least_one_alnum = true;

        while (true) {
            gunichar nuc;

            if (!string_first_char(&code_lexer->code, &nuc)) {
                return false;
            }

            if (g_unichar_isalnum(nuc)) {
                at_least_one_alnum = true;
                string_pop_char(&code_lexer->code);
                continue;
            }

            if (nuc == '.') {
                if (at_least_one_alnum) {
                    at_least_one_alnum = false;
                    string_pop_char(&code_lexer->code, NULL);
                    continue;
                }
            }
        }

        buf.len = code_lexer->code.data - buf.data;

        for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
            if (string_equals(&buf, KeywordValues[kw])) {
                code_lexer->token.type = TOKEN_KEYWORD;
                code_lexer->token.as.keyword = kw;
                return true;
            }
        }

        code_lexer->token.type = TOKEN_IDENTIFIER;
        code_lexer->token.as.identifier.data = buf.data;
        code_lexer->token.as.identifier.len = buf.len;
        code_lexer->code.data = buf.data + buf.len;
        code_lexer->code.len -= buf.len;
        return true;
    }

    if (uc == '\'' || uc == '`' || uc == '"') {
        gchar *string_start = buf.data;
        gchar *string_end = string_find(&code_lexer->code, uc);

        if (!string_end) {
            return false;
        }

        string_start = g_utf8_next_char(string_start);

        if (!string_start) {
            return false;
        }

        if (string_start > string_end) {
            return false;
        }

        code_lexer->token.type = TOKEN_STRING;
        code_lexer->token.as.string.data = string_start;
        code_lexer->token.as.string.len = string_end - string_start;

        string_end = g_utf8_next_char(string_end);

        buf.len = string_end - string_start;

        if (!string_end) {
            code_lexer->code.data = NULL;
            code_lexer->code.len = 0;
        }
        else {
            code_lexer->code.data = buf.data + buf.len;
            code_lexer->code.len -= buf.len;
        }

        return true;
    }

    if (uc == '=') {
        if (string_pop_char_if_equals(code_lexer->code, '=')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_EQUAL;
        }
        else {
            code_lexer->token.type = TOKEN_UNKNOWN;
            code_lexer->token.as.literal = uc;
        }
        return true;
    }

    if (uc == '<') {
        if (string_pop_char_if_equals(code_lexer->code, '=')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_LESS_THAN_OR_EQUAL;
        }
        else {
            code_lexer->token.type = TOKEN_UNARY_BOOLOP;
            code_lexer->token.as.unary_bool_op = UBOOLOP_LESS_THAN;
        }
        return true;
    }

    if (uc == '>') {
        if (string_pop_char_if_equals(code_lexer->code, '=')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_GREATER_THAN_OR_EQUAL;
        }
        else {
            code_lexer->token.type = TOKEN_UNARY_BOOLOP;
            code_lexer->token.as.unary_bool_op = UBOOLOP_GREATER_THAN;
        }
        return true;
    }

    if (uc == '&') {
        if (string_pop_char_if_equals(code_lexer->code, '&')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_AnD;
        }
        else {
            code_lexer->token.type = TOKEN_UNKNOWN;
            code_lexer->token.as.literal = uc;
        }
        return true;
    }

    if (uc == '|') {
        if (string_pop_char_if_equals(code_lexer->code, '|')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_OR;
        }
        else {
            code_lexer->token.type = TOKEN_UNKNOWN;
            code_lexer->token.as.literal = uc;
        }
        return true;
    }

    if (uc == '!') {
        if (string_pop_char_if_equals(code_lexer->code, '=')) {
            code_lexer->token.type = TOKEN_BOOLOP;
            code_lexer->token.as.bool_op = BOOLOP_NOT_EQUAL;
        }
        else {
            code_lexer->token.type = TOKEN_UNARY_BOOLOP;
            code_lexer->token.as.unary_bool_op = UBOOLOP_NOT;
        }
        return true;
    }

    for (MathOp m = MATHOP_FIRST; m < MATHOP_MAX; m++) {
        if (uc == MathOpValues[m]) {
            code_lexer->token.type = TOKEN_MATHOP;
            code_lexer->token.as.math_op = m;
            return true;
        }
    }

    for (Symbol s = SYMBOL_FIRST; s < SYMBOL_MAX; s++) {
        if (uc == SymbolValues[s]) {
            code_lexer->token.type = TOKEN_SYMBOL;
            code_lexer->token.as.symbol = s;
            return true;
        }
    }

    for (Whitespace ws = WHITESPACE_FIRST; ws < WHITESPACE_SPACE; ws++) {
        if (uc == WhitespaceValues[ws]) {
            code_lexer->token.type = TOKEN_WHITESPACE;
            code_lexer->token.as.whitespace = ws;
            return true;
        }
    }

    code_lexer->token.type = TOKEN_UNKNOWN;
    code_lexer->token.as.literal = uc
    return true;
}

/* vi: set et ts=4 sw=4: */

