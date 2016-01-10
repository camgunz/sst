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
 *   - ==, !-, >, >=, <, <=, ||, &&
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
