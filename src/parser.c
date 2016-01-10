#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

/*
 * Operations:
 *   - Initialize
 *   - Validate (all "expects" are satisfied)
 *   - Render
 */

void parser_init(Parser *parser, String *code) {
    lexer_init(&Parser.lexer, code);
}

ParserStatus validate_number(Parser *parser) {
    /*
     * We expect:
     *   - Close parenthesis
     *   - EOF
     *   - Comma
     *   - Whitespace
     *     - Math Op
     */
}

ParserStatus validate_keyword(Parser *parser) {
    switch (parser->lexer.token.as.keyword) {
        case KEYWORD_INCLUDE: {
            /* whitespace, string (valid path) */
        }
        case KEYWORD_IF: {
            /* Whitespace, conditional
        }
        case KEYWORD_ELIF: {
        }
        case KEYWORD_ELSE: {
        }
        case KEYWORD_ENDIF: {
        }
        case KEYWORD_FOR: {
        }
        case KEYWORD_IN: {
        }
        case KEYWORD_ENDFOR: {
        }
        case KEYWORD_RAW: {
        }
        case KEYWORD_ENDRAW: {
        }
        case KEYWORD_RANGE: {
        }
        default: {
            return PARSER_UNKNOWN_TOKEN;
        }
    }
}

ParserStatus validate_identifier(Parser *parser) {
}

ParserStatus validate_string(Parser *parser) {
}

ParserStatus validate_boolop(Parser *parser) {
}

ParserStatus validate_unary_boolop(Parser *parser) {
}

ParserStatus validate_mathop(Parser *parser) {
}

ParserStatus validate_symbol(Parser *parser) {
}

ParserStatus validate_whitespace(Parser *parser) {
}

ParserStatus parser_validate(Parser *parser) {
    LexerStatus ls;
    
    while (true) {
        ls = lexer_load_next(&parser->lexer);

        switch (ls) {
            case LEXER_INTERNAL_ERROR:
                return PARSER_INTERNAL_ERROR;
            case LEXER_UNKNOWN_TOKEN:
                return PARSER_UNKNOWN_TOKEN;
        }

        switch (parser->lexer.token.type) {
            case TOKEN_NUMBER:
                return validate_number(parser);
            case TOKEN_KEYWORD:
                return validate_keyword(parser);
            case TOKEN_IDENTIFIER:
                return validate_identifier(parser);
            case TOKEN_STRING:
                return validate_string(parser);
            case TOKEN_BOOLOP:
                return validate_boolop(parser);
            case TOKEN_UNARY_BOOLOP:
                return validate_unary_boolop(parser);
            case TOKEN_MATHOP:
                return validate_mathop(parser);
            case TOKEN_SYMBOL:
                return validate_symbol(parser);
            case TOKEN_WHITESPACE:
                return validate_whitespace(parser);
            default: {
                return PARSER_UNKNOWN_TOKEN;
            }
        }
    }
}

/* vi: set et ts=4 sw=4: */

