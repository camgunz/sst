#ifndef PARSER_H__
#define PARSER_H__

typedef enum {
    BOOLOP_NOT
} UnaryBoolOp;

typedef enum {
    BOOLOP_EQUALS,
    BOOLOP_NOT_EQUALS,
    BOOLOP_GREATER_THAN,
    BOOLOP_GREATHER_THAN_OR_EQUALS,
    BOOLOP_LESS_THAN,
    BOOLOP_LESS_THAN_OR_EQUALS,
    BOOLOP_AND,
    BOOLOP_OR,
} BoolOp;

typedef enum {
    MATHOP_ADD,
    MATHOP_SUBTRACT,
    MATHOP_MULTIPLY,
    MATHOP_DIVIDE,
    MATHOP_REMAINDER,
    MATHOP_EXPONENT
} MathOp;

typedef enum {
    SYMBOL_OPEN_BRACE,
    SYMBOL_CLOSE_BRACE,
    SYMBOL_MAX
} Symbol;

typedef enum {
    KEYWORD_INCLUDE,
    KEYWORD_IF,
    KEYWORD_ELIF,
    KEYWORD_ELSE,
    KEYWORD_ENDIF,
    KEYWORD_FOR,
    KEYWORD_IN,
    KEYWORD_ENDFOR,
    KEYWORD_RAW,
    KEYWORD_ENDRAW
    KEYWORD_MAX
} Keyword;

typedef String TextBlock;

/*
 * {{ include "path/to/template.html" }}
 */
typedef struct {
    String literal;
    String path;
} IncludeTag;

/*
 * {{ site.name }}
 * {{ site.name | capitalize }}
 */
typedef struct {
    String expression;
    String filter_name; /* optional */
} ExpressionTag;

/*
 * {{ if site.name == "Super Site" }}
 * {{ if person.is_pilot }}
 * {{ elif !person.is_driver }}
 * {{ else }}
 */
typedef struct {
    String literal;
    String block;
    BoolOp operator;    /* optional */
    String expression1; /* optional */
    String expression2; /* optional */
} ConditionalTag;

/*
 * {{ for fruit in fruits }}
 * {{ for name, age in people_to_ages }}
 */
typedef struct {
    String literal;
    String block;
    String identifiers;
    String expression;
} IterationTag;

/*
 * {{ raw }}
 */
typedef struct {
    String literal;
    String block;
} RawTag;

extern const char SymbolValues[SYMBOL_MAX];
extern const char KeywordValues[KEYWORD_MAX];

#endif

