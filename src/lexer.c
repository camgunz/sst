#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <gmp.h>
#include <mpfr.h>

#include "config.h"
#include "utils.h"
#include "str.h"
#include "lexer.h"

const uint32_t MathOpValues[MATHOP_MAX] = {
    '+', '-', '*', '/', '%', '^',
};

const uint32_t SymbolValues[SYMBOL_MAX] = {
    '(', ')', '[', ']', '{', '}', ',', '.', '\'', '`', '"', '|'
};

const uint32_t WhitespaceValues[WHITESPACE_MAX] = {
    ' ', '\t', '\r', '\n',
};

const char *BoolOpValues[BOOLOP_MAX] = {
    "==", "!=", ">", ">=", "<", "<=", "&&", "||"
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include", "if", "elif", "else", "endif", "for", "in", "endfor", "raw",
    "endraw", "range"
};

static bool seek_to_next_code_tag_start(String *s) {
    String cursor;
    StringStatus res;

    string_shallow_copy(&cursor, s);

    while (true) {
        if (string_starts_with(&cursor, "{{")) {
            break;
        }

        if (string_advance_rune(&cursor) != STRING_OK) {
            return false;
        }
    }

    string_shallow_copy(s, &cursor);

    return true;
}

static bool find_next_code_tag(String *data, String *tag) {
    String tag2;
    char *open;

    string_shallow_copy(&tag2, data);

    if (!seek_to_next_code_tag_start(&tag2)) {
        return false;
    }

    open = tag2.data;

    while (true) {
        StringStatus res;
        rune in_string = '\0';
        rune r;

        res = string_pop_rune(&tag2, &r);

        if (res != STRING_OK) {
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

        if ((in_string == '\0') && string_starts_with(&tag2, "}}")) {
            if (string_advance_rune(&tag2) != STRING_OK) {
                return false;
            }

            if (string_advance_rune(&tag2) != STRING_OK) {
                return false;
            }

            tag2.len = tag.data - open;
            tag2.data = open;

            string_shallow_copy(tag, &tag2);

            return true;
        }
    }
}

void lexer_clear(Lexer *lexer) {
    string_clear(&lexer->data);
    string_clear(&lexer->tag);
    lexer->token.type = TOKEN_UNKNOWN;
}

void lexer_set_data(Lexer *lexer, String *data) {
    lexer_clear(lexer);

    string_shallow_copy(&lexer->data, data);
}

LexerStatus lexer_base_load_next(Lexer *lexer, bool skip_whitespace) {
    String buf;
    rune r;
    StringStatus res;

    buf.data = lexer->code.data;
    buf.len = 0;

    while (true) {
        if (string_empty(&lexer->code)) {
            return LEXER_END;
        }

        if (!string_pop_char(&lexer->code, &r)) {
            return LEXER_END;
        }

        bool found_whitespace = false;

        for (Whitespace ws = WHITESPACE_FIRST; ws < WHITESPACE_MAX; ws++) {
            if (r == WhitespaceValues[ws]) {
                lexer->token.type = TOKEN_WHITESPACE;
                lexer->token.as.whitespace = ws;
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
    }

    if (rune_is_digit(r) || r == '-' || r == '.') {
        bool found_at_least_one_digit = false;
        bool found_period = false;

        if (r == '.') {
            found_period = true;
        }

        while (true) {
            rune r2;

            if (!string_first_char(&lexer->code, &r2)) {
                return LEXER_END;
            }

            if (rune_is_digit(r2)) {
                found_at_least_one_digit = true;
                string_pop_char(&lexer->code, NULL);
                continue;
            }

            if (r2 == ',') {
                string_pop_char(&lexer->code, NULL);
                continue;
            }

            if (r2 == '.') {
                if (!found_period) {
                    found_period = true;
                    string_pop_char(&lexer->code, NULL);
                    continue;
                }
            }

            break;
        }

        if (found_at_least_one_digit) {
            char *resume;

            lexer->token.type = TOKEN_NUMBER;
            mpfr_init2(lexer->token.as.number, DEFAULT_PRECISION);

            mpfr_strtofr(
                lexer->token.as.number, buf.data, &resume, 0, MPFR_RNDZ
            );

            if (resume == buf.data) {
                return LEXER_INVALID_NUMBER_FORMAT;
            }

            return string_advance_bytes(resume - buf.data);
        }
    }

    if (rune_is_alpha(r) || r == '_') {
        res = string_truncate_at_whitespace(&buf);

        if (res != STRING_OK) {
            return res;
        }

        if (!validate_identifier(&buf)) {
            return LEXER_INVALID_IDENTIFIER_FORMAT;
        }

        for (Keyword kw = KEYWORD_FIRST; kw < KEYWORD_MAX; kw++) {
            if (string_equals(&buf, KeywordValues[kw])) {
                lexer->token.type = TOKEN_KEYWORD;
                lexer->token.as.keyword = kw;

                return LEXER_OK;
            }
        }

        lexer->token.type = TOKEN_IDENTIFIER;
        string_shallow_copy(&lexer->token.as.identifier, &buf);

        return LEXER_OK;
    }

    if (r == '\'' || r == '`' || r == '"') {
        res = string_truncate_at(&buf, r);

        switch (res) {
            case STRING_END:
            case STRING_MEMORY_EXHAUSTED:
            case STRING_NOT_ASSIGNED:
            case STRING_INVALID_OPTS: {
                return res;
            }
            default: {
                break;
            }
        }

        res = string_truncate_runes(&buf, 1);

        switch (res) {
            case STRING_END:
            case STRING_MEMORY_EXHAUSTED:
            case STRING_NOT_ASSIGNED:
            case STRING_INVALID_OPTS: {
                return res;
            }
            default: {
                break;
            }
        }

        lexer->token.type = TOKEN_STRING;
        string_shallow_copy(&lexer->token.as.string, &buf);

        return string_advance_bytes(&lexer->data, buf.len + 1);
    }

    if (uc == '=') {
        if (string_pop_char_if_equals(&lexer->code, '=')) {
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
        if (string_pop_char_if_equals(&lexer->code, '=')) {
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
        if (string_pop_char_if_equals(&lexer->code, '=')) {
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
        if (string_pop_char_if_equals(&lexer->code, '&')) {
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
        if (string_pop_char_if_equals(&lexer->code, '|')) {
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
        if (string_pop_char_if_equals(&lexer->code, '=')) {
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

    lexer->token.type = TOKEN_UNKNOWN;
    lexer->token.as.literal = uc;
    return LEXER_UNKNOWN_TOKEN;
}

/* vi: set et ts=4 sw=4: */

