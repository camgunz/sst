#ifndef LANG_H__
#define LANG_H__

typedef enum {
    SYMBOL_OPAREN,
    SYMBOL_CPAREN,
    SYMBOL_EXCLAMATION_POINT,
    SYMBOL_PLUS,
    SYMBOL_MINUS,
    SYMBOL_ASTERISK,
    SYMBOL_FORWARD_SLASH,
    SYMBOL_PERCENT,
    SYMBOL_CARET,
    SYMBOL_COMMA,
    SYMBOL_OBRACKET,
    SYMBOL_CBRACKET,
    SYMBOL_PERIOD,
    SYMBOL_AND,
    SYMBOL_OR,
    SYMBOL_EQUAL,
    SYMBOL_NOT_EQUAL,
    SYMBOL_LESS_THAN_OR_EQUAL,
    SYMBOL_LESS_THAN,
    SYMBOL_GREATER_THAN_OR_EQUAL,
    SYMBOL_GREATER_THAN,
    SYMBOL_MAX
} Symbol;

#define SYMBOL_FIRST SYMBOL_OPAREN

typedef enum {
    WHITESPACE_SPACE,
    WHITESPACE_TAB,
    WHITESPACE_NEWLINE,
    WHITESPACE_MAX
} Whitespace;

#define WHITESPACE_FIRST WHITESPACE_SPACE

typedef enum {
    OP_OPAREN,
    OP_CPAREN,
    OP_BOOL_OR,
    OP_BOOL_AND,
    OP_BOOL_NOT,
    OP_BOOL_LESS_THAN,
    OP_BOOL_LESS_THAN_OR_EQUAL,
    OP_BOOL_GREATER_THAN,
    OP_BOOL_GREATER_THAN_OR_EQUAL,
    OP_BOOL_NOT_EQUAL,
    OP_BOOL_EQUAL,
    OP_MATH_ADD,
    OP_MATH_SUBTRACT,
    OP_MATH_MULTIPLY,
    OP_MATH_DIVIDE,
    OP_MATH_REMAINDER,
    OP_MATH_POSITIVE,
    OP_MATH_NEGATIVE,
    OP_MATH_EXPONENT,
    OP_MAX
} Operator;

#define OP_FIRST OP_MATH_ADD

typedef enum {
    OP_CATEGORY_UNARY_BOOL,
    OP_CATEGORY_BINARY_BOOL,
    OP_CATEGORY_UNARY_MATH,
    OP_CATEGORY_BINARY_MATH,
    OP_CATEGORY_PAREN,
} OperatorCategory;

typedef enum {
    KEYWORD_INCLUDE,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_ENDIF,
    KEYWORD_FOR,
    KEYWORD_IN,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_ENDFOR,
    KEYWORD_RAW,
    KEYWORD_ENDRAW,
    KEYWORD_MAX
} Keyword;

#define KEYWORD_FIRST KEYWORD_INCLUDE

typedef enum {
    OP_ASSOC_NONE,
    OP_ASSOC_LEFT,
    OP_ASSOC_RIGHT,
} OperatorAssociativity;

typedef struct {
    const char *value;
    OperatorAssociativity assoc;
    unsigned char prec;
    unsigned char arity;
} OperatorInformation;

extern OperatorInformation OperatorInfo[OP_MAX];
extern const char *KeywordValues[KEYWORD_MAX];
extern const char *SymbolValues[SYMBOL_MAX];

#endif

/* vi: set et ts=4 sw=4: */
