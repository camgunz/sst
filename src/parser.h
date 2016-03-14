#ifndef PARSER_H__
#define PARSER_H__

typedef enum {
    PARSER_OK                    = LEXER_OK,
    PARSER_DATA_MEMORY_EXHAUSTED = LEXER_DATA_MEMORY_EXHAUSTED,
    PARSER_DATA_OVERFLOW         = LEXER_DATA_OVERFLOW,
    PARSER_DATA_INVALID_UTF8     = LEXER_DATA_INVALID_UTF8,
    PARSER_DATA_NOT_ASSIGNED     = LEXER_DATA_NOT_ASSIGNED,
    PARSER_DATA_INVALID_OPTS     = LEXER_DATA_INVALID_OPTS,
    PARSER_END                   = LEXER_END,
    PARSER_UNKNOWN_TOKEN         = LEXER_UNKNOWN_TOKEN,
    PARSER_INVALID_NUMBER_FORMAT = LEXER_INVALID_NUMBER_FORMAT,
    PARSER_INTERNAL_ERROR,
    PARSER_UNEXPECTED_TOKEN,
    PARSER_NESTED_EXPRESSION,
    PARSER_MAX
} ParserStatus;

typedef struct {
    Lexer lexer;
    Block block;
} Parser;

void         parser_init(Parser *parser, SSlice *code);
ParserStatus parser_load_next(Parser *parser);

#endif

/* vi: set et ts=4 sw=4: */

