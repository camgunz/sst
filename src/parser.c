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
#include "lexer.h"
#include "parser.h"

#define TOKEN_ALLOC_COUNT 1000

static ParserStatus parse_include(Parser *parser) {
    LexerStatus  ls;
    Token       *token;

    ls = lexer_load_next_skip_whitespace(&parser->lexer);

    if (ls != LEXER_OK) {
        return ls;
    }

    token = lexer_get_current_token(&parser->lexer);

    if (!token) {
        return PARSER_INTERNAL_ERROR;
    }

    if (token->type != TOKEN_STRING) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    parser->block.type = BLOCK_INCLUDE;
    sslice_shallow_copy(&parser->block.as.include.path, &token->as.string);

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

static ParserStatus parse_paren(Parser *parser) {
    return PARSER_OK;
}

void parser_init(Parser *parser, SSlice *data) {
    parser->parenthesis_level = 0;
    parser->bracket_level = 0;
    parser->brace_level = 0;
    lexer_init(&parser->lexer);
    lexer_set_data(&parser->lexer, data);
}

void parser_clear(Parser *parser) {
    parser->parenthesis_level = 0;
    parser->bracket_level = 0;
    parser->brace_level = 0;
    lexer_clear(&parser->lexer);
}

ParserStatus parser_load_next(Parser *parser) {
    LexerStatus   ls;
    Token        *token;

    ls = lexer_load_next_skip_whitespace(&parser->lexer);

    switch (ls) {
        case LEXER_DATA_MEMORY_EXHAUSTED:
            return PARSER_DATA_MEMORY_EXHAUSTED;
        case LEXER_DATA_OVERFLOW:
            return PARSER_DATA_OVERFLOW;
        case LEXER_DATA_INVALID_UTF8:
            return PARSER_DATA_INVALID_UTF8;
        case LEXER_DATA_NOT_ASSIGNED:
            return PARSER_DATA_NOT_ASSIGNED;
        case LEXER_DATA_INVALID_OPTS:
            return PARSER_DATA_INVALID_OPTS;
        case LEXER_END:
            return PARSER_END;
        case LEXER_UNKNOWN_TOKEN:
            return PARSER_UNKNOWN_TOKEN;
        case LEXER_INVALID_NUMBER_FORMAT:
            return PARSER_INVALID_NUMBER_FORMAT;
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

    token = lexer_get_current_token(&parser->lexer);

    if (!token) {
        return PARSER_END;
    }

    switch (token->type) {
         case TOKEN_SYMBOL: {
            switch (token->as.symbol) {
                case SYMBOL_OPAREN: {
                    return parse_paren(parser);
                }
                default: {
                    break;
                }
            }
            break;
        }
        case TOKEN_KEYWORD: {
            switch (token->as.keyword) {
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
                default: {
                    break;
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
        case TOKEN_WHITESPACE:
        default: {
            break;
        }
    }

    return PARSER_UNEXPECTED_TOKEN;
}

/* vi: set et ts=4 sw=4: */

