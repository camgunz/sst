#ifndef LEXER_H__
#define LEXER_H__

typedef enum {
    MATHOP_ADD,
    MATHOP_SUBTRACT,
    MATHOP_MULTIPLY,
    MATHOP_DIVIDE,
    MATHOP_REMAINDER,
    MATHOP_EXPONENT,
    MATHOP_MAX
} MathOp;

#define MATHOP_FIRST MATHOP_ADD

typedef enum {
    SYMBOL_OPAREN,
    SYMBOL_CPAREN,
    SYMBOL_OBRACKET,
    SYMBOL_CBRACKET,
    SYMBOL_OBRACE,
    SYMBOL_CBRACE,
    SYMBOL_COMMA,
    SYMBOL_PERIOD,
    SYMBOL_SINGLE_QUOTE,
    SYMBOL_BACK_QUOTE,
    SYMBOL_DOUBLE_QUOTE,
    SYMBOL_PIPE,
    SYMBOL_MAX
} Symbol;

#define SYMBOL_FIRST SYMBOL_OPAREN

typedef enum {
    WHITESPACE_SPACE,
    WHITESPACE_TAB,
    WHITESPACE_CARRIAGE_RETURN,
    WHITESPACE_NEWLINE,
    WHITESPACE_MAX
} Whitespace;

#define WHITESPACE_FIRST WHITESPACE_SPACE

typedef enum {
    UBOOLOP_NOT,
    UBOOLOP_MAX,
} UnaryBoolOp;

#define UBOOLOP_FIRST UBOOLOP_NOT

typedef enum {
    BOOLOP_EQUAL,
    BOOLOP_NOT_EQUAL,
    BOOLOP_GREATER_THAN,
    BOOLOP_GREATER_THAN_OR_EQUAL,
    BOOLOP_LESS_THAN,
    BOOLOP_LESS_THAN_OR_EQUAL,
    BOOLOP_AND,
    BOOLOP_OR,
    BOOLOP_MAX
} BoolOp;

#define BOOLOP_FIRST BOOLOP_EQUAL

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
    KEYWORD_ENDRAW,
    KEYWORD_RANGE,
    KEYWORD_MAX
} Keyword;

#define KEYWORD_FIRST KEYWORD_INCLUDE

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_TEXT,
    TOKEN_NUMBER,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_BOOLOP,
    TOKEN_UNARY_BOOLOP,
    TOKEN_MATHOP,
    TOKEN_SYMBOL,
    TOKEN_WHITESPACE,
    TOKEN_MAX
} TokenType;

typedef enum {
    LEXER_OK = STRING_OK,
    LEXER_DATA_MEMORY_EXHAUSTED = STRING_MEMORY_EXHAUSTED,
    LEXER_DATA_OVERFLOW = STRING_OVERFLOW,
    LEXER_DATA_INVALID_UTF8 = STRING_INVALID_UTF8,
    LEXER_DATA_NOT_ASSIGNED = STRING_NOT_ASSIGNED,
    LEXER_DATA_INVALID_OPTS = STRING_INVALID_OPTS,
    LEXER_END = STRING_END,
    LEXER_UNKNOWN_TOKEN,
    LEXER_INVALID_NUMBER_FORMAT,
    LEXER_MAX
} LexerStatus;

typedef struct {
    TokenType type;
    union {
        String      text;
        rune        literal;
        mpfr_t      number;
        String      identifier;
        String      string;
        MathOp      math_op;
        Symbol      symbol;
        Whitespace  whitespace;
        UnaryBoolOp unary_bool_op;
        BoolOp      bool_op;
        Keyword     keyword;
    } as;
} Token;

extern const rune  MathOpValues[MATHOP_MAX];
extern const rune  SymbolValues[SYMBOL_MAX];
extern const rune  WhitespaceValues[WHITESPACE_MAX];
extern const char *BoolOpValues[BOOLOP_MAX];
extern const char *KeywordValues[KEYWORD_MAX];

typedef struct {
    String data;
    String tag;
    Token  token;
} Lexer;

void        lexer_clear(Lexer *lexer);
void        lexer_set_data(Lexer *lexer, String *data);
LexerStatus lexer_base_load_next(Lexer *lexer, bool skip_whitespace);

#define lexer_load_next(lexer) \
    lexer_base_load_next(lexer, false)

#define lexer_load_next_skip_whitespace(lexer) \
    lexer_base_load_next(lexer, true)

#endif

