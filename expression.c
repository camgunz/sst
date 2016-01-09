/*
 * Expressions:
 *
 * Numeric literal:
 *   - integer
 *   - decimal
 *   - strip out commas
 *   - all math uses bignums (figure out a library for this)
 *
 * String literal:
 *   - quote type (', `, ")
 *
 * Sequence literals:
 *   - []
 *
 * Range literals:
 *   - [, ]: inclusive
 *   - (, ): exclusive
 *   - Math expressions as arguments, 1, 2 or 3 arguments total.
 *   - Also accept '..' for the second argument
 *
 * Identifier:
 *   - Alphanumeric (no restrictions on position)
 *     - If there is no alphabetical member, it's a numeric literal instead
 *   - First member cannot be:
 *     - '
 *     - `
 *     - "
 *     - [
 *     - (
 *
 * Math operator:
 *   - +, -, *, /, %, ^
 *
 * Unary boolean operator:
 *   - !
 * Boolean operator:
 *   - ==, !-, >, >=, <, <=, ||, &&
 *
 * Math expressions:
 *   - First operand (numeric literal or identifier)
 *   - operator
 *   - Second operand (numeric literal or identifier)
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
