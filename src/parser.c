#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "parser.h"

#define eof(status) status_failure( \
    status,                         \
    "parser",                       \
    PARSER_EOF,                     \
    "EOF"                           \
)

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
    (void)parser;
    (void)status;
    return status_ok(status);
}

/* vi: set et ts=4 sw=4: */
