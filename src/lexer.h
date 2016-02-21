#ifndef LEXER_H__
#define LEXER_H__

#define TOKEN_QUEUE_SIZE 3

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
    LEXER_OK                    = SSLICE_OK,
    LEXER_DATA_MEMORY_EXHAUSTED = SSLICE_MEMORY_EXHAUSTED,
    LEXER_DATA_OVERFLOW         = SSLICE_OVERFLOW,
    LEXER_DATA_INVALID_UTF8     = SSLICE_INVALID_UTF8,
    LEXER_DATA_NOT_ASSIGNED     = SSLICE_NOT_ASSIGNED,
    LEXER_DATA_INVALID_OPTS     = SSLICE_INVALID_OPTS,
    LEXER_END                   = SSLICE_END,
    LEXER_UNKNOWN_TOKEN,
    LEXER_INVALID_NUMBER_FORMAT,
    LEXER_MAX
} LexerStatus;

typedef struct {
    TokenType type;
    union {
        SSlice       text;
        rune         literal;
        mpd_t       *number;
        SSlice       identifier;
        SSlice       string;
        MathOp       math_op;
        Symbol       symbol;
        Whitespace   whitespace;
        UnaryBoolOp  unary_bool_op;
        BoolOp       bool_op;
        Keyword      keyword;
    } as;
} Token;

extern const rune  MathOpValues[MATHOP_MAX];
extern const rune  SymbolValues[SYMBOL_MAX];
extern const rune  WhitespaceValues[WHITESPACE_MAX];
extern const char *BoolOpValues[BOOLOP_MAX];
extern const char *KeywordValues[KEYWORD_MAX];
extern const char *TokenTypes[TOKEN_MAX];

typedef struct {
    uint8_t head;
    uint8_t tail;
    Token tokens[TOKEN_QUEUE_SIZE];
} TokenQueue;

typedef struct {
    SSlice         data;
    SSlice         tag;
    bool           in_code;
    TokenQueue     tokens;
    mpd_context_t  mpd_ctx;
} Lexer;

LexerStatus token_clear(Token *token);
LexerStatus token_copy(Token *dst, Token *src);
char*       token_to_string(Token *token);

void        token_queue_clear(TokenQueue *token_queue);
uint8_t     token_queue_count(TokenQueue *token_queue);
bool        token_queue_empty(TokenQueue *token_queue);
bool        token_queue_full(TokenQueue *token_queue);
bool        token_queue_push(TokenQueue *token_queue, Token *token);
Token*      token_queue_pop(TokenQueue *token_queue);
Token*      token_queue_push_new(TokenQueue *token_queue);

void        lexer_init(Lexer *lexer);
void        lexer_clear(Lexer *lexer);
void        lexer_set_data(Lexer *lexer, SSlice *data);
LexerStatus lexer_load_next(Lexer *lexer);
Token*      lexer_get_previous_token(Lexer *lexer);
Token*      lexer_get_previous_token(Lexer *lexer);
Token*      lexer_get_current_token(Lexer *lexer);

#endif

/* vi: set et ts=4 sw=4: */

