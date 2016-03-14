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

/* vi: set et ts=4 sw=4: */

