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

#define TOKEN_ALLOC_COUNT 1000

static ParserStatus load_next_token(Parser *parser, Token **token) {
    LexerStatus lstatus = lexer_load_next(&parser->lexer);

    if (lstatus != LEXER_OK) {
        return lstatus;
    }

    *token = lexer_get_current_token(&parser->lexer);

    if (!*token) {
        return PARSER_END;
    }

    return PARSER_OK;
}

static ParserStatus load_expecting_symbol(Parser *parser, Symbol sym) {
    ParserStatus  pstatus;
    Token        *token = NULL;

    pstatus = load_next_token(parser, &token);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    if (token->type != TOKEN_SYMBOL) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    if (token->as.symbol != sym) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    return PARSER_OK;
}

static ParserStatus load_expecting_whitespace(Parser *parser, Whitespace ws) {
    ParserStatus  pstatus;
    Token        *token = NULL;

    pstatus = load_next_token(parser, &token);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    if (token->type != TOKEN_WHITESPACE) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    if (token->as.whitespace != ws) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    return PARSER_OK;
}

static ParserStatus load_expecting_string(Parser *parser, Token **token) {
    ParserStatus  pstatus;
    Token        *next_token = NULL;

    pstatus = load_next_token(parser, &next_token);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    if (next_token->type != TOKEN_STRING) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    *token = next_token;

    return PARSER_OK;
}

static ParserStatus parse_include(Parser *parser) {
    ParserStatus pstatus;
    Token        *token = NULL;

    pstatus = load_expecting_whitespace(parser, WHITESPACE_SPACE);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    pstatus = load_expecting_string(parser, &token);

    if (pstatus != PARSER_OK) {
        return pstatus;
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
    SSliceStatus  sstatus;
    LexerStatus   lstatus;
    ParserStatus  pstatus;
    Token        *token = NULL;

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

    pstatus = load_next_token(parser, &token);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    if (token->type == TOKEN_TEXT) {
        parser->block.type = BLOCK_TEXT;
        parser->block.as.text = token->as.text;

        return PARSER_OK;
    }

    if (token->type != TOKEN_SYMBOL) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    if (token->as.symbol != SYMBOL_OBRACE) {
        return PARSER_UNEXPECTED_TOKEN;
    }

    pstatus = load_expecting_symbol(parser, SYMBOL_OBRACE);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    pstatus = load_expecting_whitespace(parser, WHITESPACE_SPACE);

    pstatus = load_next_token(parser, &token);

    if (pstatus != PARSER_OK) {
        return pstatus;
    }

    /*
     * Math expression
     *   - Number: {{ 1 + 108 }}
     *   - Number: {{ -1 + 108 }}
     *   - Number: {{ person.age + 10 }}
     *   - Number: {{ -person.age + 10 }}
     *   - Paren: {{ (1 / 14) + ((87 + 3) / 2) }}
     *   - Paren: {{ -(1 / 14) + ((87 + 3) / 2) }}
     *   - Paren: {{ (person.age / 14) + ((87 + 3) / 2) }}
     *   - Paren: {{ -(person.age / 14) + ((87 + 3) / 2) }}
     * Include statement
     * If statement
     * For statement
     */

    printf("Token type: %d\n", token->type);

    switch (token->type) {
        case TOKEN_KEYWORD:
            switch (token->as.keyword) {
                case KEYWORD_INCLUDE:
                    return parse_include(parser);
                case KEYWORD_IF:
                    return parse_conditional(parser);
                case KEYWORD_FOR:
                    return parse_iteration(parser);
                default:
                    return PARSER_UNEXPECTED_TOKEN;
            }
        default:
            return parse_math_expression(parser);
    }

    return PARSER_UNEXPECTED_TOKEN;
}

/* vi: set et ts=4 sw=4: */

