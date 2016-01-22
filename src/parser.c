#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gmp.h>
#include <mpfr.h>

#include "str.h"
#include "splitter.h"
#include "lexer.h"
#include "parser.h"

#define TOKEN_ALLOC_COUNT 1000

static ParserStatus parse_code(Parser *parser) {
    ParserStatus ps;
    LexerStatus  ls;

    lexer_init(&parser->lexer, &parser->splitter.section);
    ls = lexer_load_next_skip_whitespace(&parser->lexer);

    switch (ls) {
        case LEXER_EOF:
            return PARSER_EOF;
        case LEXER_INTERNAL_ERROR:
            return PARSER_INTERNAL_ERROR;
        case LEXER_UNKNOWN_TOKEN:
            return PARSER_UNKNOWN_TOKEN;
        case LEXER_OK:
        case LEXER_MAX:
        default:
            break;
    }

    /*
     * SYMBOL_OPAREN:
     *   - Has to be math or boolean expression
     *   - return parse_paren
     *     - Special case of parse_start that assumes math or boolean
     *       expression
     *     - Also probably have to check for recursion here; how though?
     *       - Find first close parenthesis
     *       - Evaluate top-level (lowest-level, actually) expression
     *         - Meaning, go back to the most recent open parenthesis and
     *           evaluate the expression
     *       - Repeat for all closing parentheses
     *         - Easy enough to do because the matching open parenthesis will
     *           always be the first one encountered moving backwards
     *         
     * KEYWORD_INCLUDE:
     *   - return parse_include
     * KEYWORD_IF:
     *   - return parse_conditional
     * KEYWORD_FOR:
     *   - return parse_iteration
     * Number:
     *   - If there is no next token:
     *     - Error
     *   - If the next token is a math op:
     *     - return parse_math_expression
     * Literal:
     *   - If there is no next token:
     *     - return a literal block
     *   - If the next token is a pipe:
     *     - evaluate the pipe
     *   - If the next token is a math op:
     *     - return parse_math_expression
     *
     */

    switch (parser->lexer.token.type) {
         case TOKEN_SYMBOL: {
            switch (parser->lexer.token.as.symbol) {
                case SYMBOL_OPAREN: {
                    return parse_paren(parser);
                }
            }
            break;
        }
        case TOKEN_KEYWORD: {
            switch (parser->lexer.token.as.keyword) {
                case KEYWORD_INCLUDE: {
                    return parse_include(parser);
                }
                case KEYWORD_IF: {
                    return parse_conditional(parser);
                }
                case KEYWORD_FOR: {
                    return parse_iteration(parser);
                }
                case KEYWORD_RAW: {
                    return validate_raw(parser);
                }
            }
            break;
        }
        case TOKEN_NUMBER: {
            return parse_math_expression(parser);
        }
        case TOKEN_IDENTIFIER: {
            /* Could be a math expression also */
            return parse_expression(parser);
        }
        case TOKEN_UNKNOWN: {
            return PARSER_UNKNOWN_TOKEN;
        }
        case TOKEN_STRING:
        case TOKEN_BOOLOP:
        case TOKEN_UNARY_BOOLOP:
        case TOKEN_MATHOP:
        case TOKEN_WHITESPACE: {
            return PARSER_UNEXPECTED_TOKEN;
        }
    }
}

void parser_init(Parser *parser, String *code) {
    splitter_init(&parser->splitter, code->data);
    parser->parenthesis_level = 0;
    parser->bracket_level = 0;
    parser->brace_level = 0;
    parser->tokens = g_array_sized_new(
        false, false, sizeof(Token), TOKEN_ALLOC_COUNT
    );
}

void parser_clear(Parser *parser) {
    splitter_clear(&parser->splitter);
    lexer_clear(&parser->lexer);
    parser->parenthesis_level = 0;
    parser->bracket_level = 0;
    parser->brace_level = 0;
    g_array_free(parser->tokens, true);
}

ParserStatus parser_load_next(Parser *parser) {
    if (!splitter_load_next(&parser->splitter)) {
        return PARSER_EOF;
    }

    if (parser->splitter.section_is_code) {
        return parse_code(parser);
    }

    parser->block.type = BLOCK_TEXT;
    string_copy(&parser->block.as.text, &parser->splitter.section);

    return PARSER_OK;
}

static ParserStatus parse_include(Parser *parser) {
    ParserStatus ps;

    ps = eat_whitespace(parser);

    if (ps != PARSER_OK) {
        return ps;
    }

    if (parser->lexer.token.type != TOKEN_STRING) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    parser->block.type = BLOCK_INCLUDE;
    string_copy(
        &parser->block.as.include.tag,
        &parser->splitter.section
    );
    string_copy(
        &parser->block.as.include.path,
        parser->lexer.token.as.string
    );

    return PARSER_OK;
}

static ParserStatus parse_conditional(Parser *parser) {
    /* {{ if person.age >= 18 }} */
    /* {{ if name == "hey there" }} */
    /* {{ if width + margin > limit }} */
    /* {{ expression, boolop, expression }} */
    return PARSER_OK;
}

static ParserStatus parse_iteration(Parser *parser) {
    return PARSER_OK;
}

static ParserStatus validate_raw(Parser *parser) {
    return PARSER_OK;
}

static ParserStatus parse_math_expression(Parser *parser) {
    return PARSER_OK;
}

static ParserStatus parse_expression(Parser *parser) {
    /* Could also be a math expression, depends on finding a math operator */
    return PARSER_OK;
}
/* vi: set et ts=4 sw=4: */

