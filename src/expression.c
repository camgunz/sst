#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "token.h"
#include "block.h"
#include "lexer.h"
#include "parser.h"

/*
 * Expression evaluation
 *
 * Expressions evaluate either to identifiers or values
 *
 * Expression: <string>
 * Expression: <number>
 * Expression: <identifier>
 * Expression: <string><addop><string>
 * Expression: <numexp><mathop><numexp>
 * Expression: <numexp><boolop><numexp>
 * Expression: <boolexp><orop><boolexp>
 * Expression: <boolexp><andop><boolexp>
 * Expression: <notop><boolexp>
 * Expression: <keyword:range><openparen><numexp>[numexp][numexp]<closeparen>
 * Expression: <openbracket>[exp]...<closebracket>
 *
 */

/*
 * ## Literals
 *
 * ### Numeric Literal
 *   - integer
 *   - decimal
 *   - strip out commas
 *   - all math uses bignums (figure out a library for this)
 *
 * ### String Literal
 *   - quote type (', `, ")
 *
 * ### Sequence literals
 *   - start with "["
 *   - end with "]"
 *   - contain 1 or more numeric or string literals, separated by ","
 *
 * ### Range literals
 *   - start with "range("
 *   - end with ")"
 *   - 1, 2 or 3 arguments total separated by ","
 *   - Arguments are math expressions
 *   - 2nd argument may also be ".."
 *   - Default 3rd argument is 1
 *
 * ### Identifier
 *   - Alphanumeric (no restrictions on position)
 *     - If there is no alphabetical member, it's a numeric literal instead
 *   - First member cannot be:
 *     - '
 *     - `
 *     - "
 *     - [
 *     - (
 */

/*
 * ## Operators
 *
 * ### Math operator
 *   - +, -, *, /, %, ^
 *
 * ### Unary boolean operator
 *   - !
 *
 * ### Boolean operator
 *   - ==, !=, >, >=, <, <=, ||, &&
 *
 * ### Math expressions
 *   - First operand (numeric literal or identifier)
 *   - operator
 *   - Second operand (numeric literal or identifier)
 */

/*
 * Expressions:
 *
 * Unary Boolean expression:
 *   - Unary boolean operator
 *   - operand (identifier)
 *
 * Boolean expression:
 *   - First operand (numeric literal, string literal, identifier)
 *   - Non-unary boolean operator
 *   - Second operand (numeric literal, string literal, identifier)
 *
 * Variable expressions:
 *   - First operand (numeric literal, string literal, identifier)
 *   - Optional processor (identifier)
 */

/*
 * Design:
 *
 * Need a way to lookup identifiers
 *   - Global state
 *
 * Need a way to parse math expressions
 *
 * Need to parse literals
 *   - numbers
 *   - strings
 *   - sequences
 *   - ranges
 */

static size_t get_oparen_area_end(SSlice *s) {
    size_t oparen_area_end = 0;

    for (size_t i = 0; i < s->len; i++) {
        oparen_area_end++;

        if (*(s->data + i) != '(') {
            break;
        }
    }

    return oparen_area_end;
}

static size_t get_cparen_area_start(SSlice *s) {
    size_t cparen_area_start = s->len - 1;

    for (size_t i = s->len - 1; i >= 0; i--) {
        cparen_area_start--;

        if (*(s->data + i) != ')') {
            break;
        }
    }

    return cparen_area_start;
}

static bool find_matching_oparen(SSlice *s, size_t cparen_index,
                                            size_t *match) {
    size_t paren_depth = 1;

    if (cparen_index == 0) {
        return false;
    }

    for (size_t i = cparen_index - 1; i >= 0; i++) {
        char c = *(s->data + i);

        if (c == ')') {
            paren_depth++;
        }
        else if (c == '(') {
            if (paren_depth == 1) {
                *match = i;
                return true;
            }
            paren_depth--;
        }
    }

    return false;
}

static SSliceStatus strip_extraneous_parentheses(SSlice *s) {
    size_t extraneous_paren_count = 0;
    size_t oparen_area_end = get_oparen_area_end(s);
    size_t cparen_area_start = get_cparen_area_start(s);

    for (size_t i = 0; i < s->len; i++) {
    }

    if (extraneous_paren_count > 0) {
        SSliceStatus res;

        res = sslice_advance_runes(s, extraneous_paren_count);

        if (res != SSLICE_OK) {
            return res;
        }

        res = sslice_truncate_runes(s, extraneous_paren_count);

        if (res != SSLICE_OK) {
            return res;
        }
    }

    return SSLICE_OK;
}

static bool find_next_split(SSlice *s, MathOp *op, SSlice *split) {
    size_t paren_depth = 0;
    SSlice ss;
    rune r;
    SSliceStatus sstatus;
    MathOp mop;
    MathOp low_op = MATHOP_MAX;
    SSlice low_s;

    sslice_shallow_copy(&ss, s);

    while (true) {
        sstatus = sslice_pop_rune(&ss, &r);

        if (sstatus != SSLICE_OK) {
            return false;
        }

        if (rune_is_whitespace(r)) {
            continue;
        }

        if (r == '(') {
            paren_depth++;
        }

        if (paren_depth > 0) {
            if (rune == ')') {
                paren_depth--;
            }

            continue;
        }

        for (mop = MATHOP_FIRST; mop < MATHOP_MAX; m++) {
            if (rune == MathOpValues[mop]) {
                break;
            }
        }

        if (mop == MATHOP_MAX) {
            continue;
        }

        if (MathOpPrecedence[mop] == 0) {
            low_op = mop;
            sslice_copy(&low_s, &ss);
            break;
        }

        if ((low_op == MATHOP_MAX) || (MathOpPrecedence[mop] < low_op)) {
            low_op = mop;
            sslice_copy(&low_s &ss);
        }
    }

    if (low_op == MATHOP_MAX) {
        return false;
    }

    *op = low_op;
    sslice_copy(split, &low_s);
    return true;
}

bool expression_from_sslice(SSlice *s, Expression **expression) {
    SSliceStatus sstatus;
    SSlice copy;
    SSlice split;
    MathOp op;

    sslice_shallow_copy(&copy, s);

    sstatus = sslice_skip_past_whitespace(&copy);

    if (sstatus != SSLICE_OK) {
        return false;
    }

    sstatus = sslice_truncate_whitespace(&copy);

    if (sstatus != SSLICE_OK) {
        return false;
    }

    if (sslice_empty(&copy)) {
        return false;
    }

    if (!find_next_split(&copy, &op, &split)) {
        return false;
    }

    sstatus = sslice_truncate_at_subslice(&copy, &split);

    if (sstatus != SSLICE_OK) {
        return false;
    }
}

/* vi: set et ts=4 sw=4: */

