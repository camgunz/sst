#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "parser.h"

#define invalid_syntax(status) status_failure( \
    status,                                    \
    "parser",                                  \
    PARSER_INVALID_SYNTAX,                     \
    "Invalid syntax"                           \
)

static bool parser_parse_keyword(Parser *parser, Status *status) {
    (void)parser;
    (void)status;

    return status_ok(status);
}

static bool parser_parse_text(Parser *parser, Status *status) {
    (void)parser;
    (void)status;

    return status_ok(status);
}

static bool parser_parse_expression(Parser *parser, Status *status) {
    (void)parser;
    (void)status;

    return status_ok(status);
}

bool parser_init(Parser *parser, SSlice *data, DecimalContext *ctx,
                                               Status *status) {
    if (!lexer_init(&parser->lexer, data, status)) {
        return false;
    }

    parser->ctx = ctx;

    return status_ok(status);
}

void parser_clear(Parser *parser) {
    lexer_clear(&parser->lexer);

    parser->ctx = NULL;
}

bool parser_load_next(Parser *parser, Status *status) {
    if (!lexer_load_next(&parser->lexer, status)) {
        return false;
    }

    switch (parser->lexer.code_token.type) {
        case CODE_TOKEN_KEYWORD:
            return parser_parse_keyword(parser, status);
        case CODE_TOKEN_TEXT:
             return parser_parse_text(parser, status);
        case CODE_TOKEN_NUMBER:
        case CODE_TOKEN_STRING:
        case CODE_TOKEN_LOOKUP:
        case CODE_TOKEN_FUNCTION_START:
        case CODE_TOKEN_INDEX_START:
        case CODE_TOKEN_ARRAY_START:
        case CODE_TOKEN_OPERATOR:
            return parser_parse_expression(parser, status);
        default:
            break;
    }

    return invalid_syntax(status);
}

/* vi: set et ts=4 sw=4: */
