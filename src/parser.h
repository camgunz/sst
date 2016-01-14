#ifndef PARSER_H__
#define PARSER_H__

typedef String Text;

/*
 * {{ include "path/to/template.html" }}
 */
typedef struct {
    String literal;
    String path;
} Include;

/*
 * {{ site.name }}
 * {{ site.name | capitalize }}
 */
typedef struct {
    String literal;
    String filter_name; /* optional */
} Expression;

/*
 * {{ if site.name == "Super Site" }}
 * {{ if person.is_pilot }}
 * {{ elif !person.is_driver }}
 * {{ else }}
 * {{ endif }}
 */
typedef struct {
    String literal;
    String block;
    BoolOp operator;    /* optional */
    String expression1; /* optional */
    String expression2; /* optional */
} Conditional;

/*
 * {{ for fruit in fruits }}
 * {{ for name, age in people_to_ages }}
 * {{ endfor }}
 */
typedef struct {
    String literal;
    String block;
    String identifiers;
    String expression;
} Iteration;

/*
 * {{ raw }}
 * {{ endraw }}
 */
typedef struct {
    String literal;
    String block;
} Raw;

typedef enum {
    PARSER_OK = LEXER_OK,
    PARSER_EOF = LEXER_EOF,
    PARSER_INTERNAL_ERROR = LEXER_INTERNAL_ERROR,
    PARSER_UNKNOWN_TOKEN = LEXER_UNKNOWN_TOKEN,
    PARSER_UNEXPECTED_CLOSE_PARENTHESIS,
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

typedef struct {
    BlockType type;
    union {
        Text text;
        Include include;
        Expression expression;
        Conditional conditional;
        Iteration iteration;
        Raw raw;
    } as;
} Block;

typedef struct {
    Lexer   lexer;
    size_t  parenthesis_level;
    size_t  bracket_level;
    size_t  brace_level;
    GArray *blocks;
} Parser;

ParserStatus parser_validate(Parser *parser);

#endif

