#ifndef PARSER_H__
#define PARSER_H__

typedef struct {
    Lexer lexer;
    DecimalContext *ctx;
} Parser;

bool parser_init(Parser *parser, SSlice *data, DecimalContext *ctx,
                                               Status *status);
void parser_clear(Parser *parser);
bool parser_load_next(Parser *parser, Status *status);

#endif

/* vi: set et ts=4 sw=4: */

