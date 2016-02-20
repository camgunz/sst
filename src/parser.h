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
    PARSER_MAX
} ParserStatus;

typedef enum {
    BLOCK_TEXT,
    BLOCK_INCLUDE,
    BLOCK_EXPRESSION,
    BLOCK_CONDITIONAL,
    BLOCK_ITERATION,
    BLOCK_RAW,
    BLOCK_MAX
} BlockType;

typedef SSlice Text;

/*
 * {{ include "path/to/template.html" }}
 */
typedef struct {
    SSlice tag;
    SSlice path;
} IncludeStatement;

typedef struct {
    BlockType type;
    union {
        Text             text;
        IncludeStatement include;
    } as;
} Block;

#if 0

typedef SSlice IdentifierExpression;

/*
 * {{ site.name }}
 * {{ site.name | capitalize }}
 */
typedef struct {
    SSlice literal;
    SSlice filter_name; /* optional */
} Expression;

/*
 * {{ if site.name == "Super Site" }}
 * {{ if person.is_pilot }}
 * {{ elif !person.is_driver }}
 * {{ else }}
 * {{ endif }}
 */
typedef struct {
    SSlice literal;
    SSlice block;
    BoolOp operator;    /* optional */
    SSlice expression1; /* optional */
    SSlice expression2; /* optional */
} ConditionalStatement;

/*
 * {{ for fruit in fruits }}
 * {{ for name, age in people_to_ages }}
 * {{ endfor }}
 */
typedef struct {
    SSlice literal;
    SSlice block;
    SSlice identifiers;
    SSlice expression;
} IterationStatement;

/*
 * {{ raw }}
 * {{ endraw }}
 */
typedef struct {
    SSlice literal;
    SSlice block;
} RawStatement;

#endif

typedef struct {
    Lexer     lexer;
    size_t    parenthesis_level;
    size_t    bracket_level;
    size_t    brace_level;
    Block     block;
} Parser;

void         parser_init(Parser *parser, SSlice *code);
ParserStatus parser_validate(Parser *parser);

#endif

/* vi: set et ts=4 sw=4: */

