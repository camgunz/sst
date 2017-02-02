#ifndef TOKENIZER_H__
#define TOKENIZER_H__

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_TEXT,
    TOKEN_RAW,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_WHITESPACE,
} TokenType;

typedef struct {
    TokenType type;
    const char *location;
    union {
        SSlice text;
        SSlice raw;
        SSlice number;
        SSlice string;
        Symbol symbol;
        Keyword keyword;
        SSlice identifier;
        Whitespace whitespace;
    } as;
} Token;

enum {
    TOKENIZER_INVALID_SYNTAX = 1,
    TOKENIZER_EOF,
    TOKENIZER_UNEXPECTED_EOF,
    TOKENIZER_INVALID_NUMBER_FORMAT,
    TOKENIZER_HANGING_RAW,
    TOKENIZER_UNKNOWN_TOKEN,
    TOKENIZER_TOKEN_NOT_HANDLED,
    TOKENIZER_INVALID_WHITESPACE,
};

typedef struct {
    SSlice *data;
    size_t line;
    size_t column;
    Token token;
    bool in_code;
} Tokenizer;

void tokenizer_init(Tokenizer *tokenizer, SSlice *data);
void tokenizer_clear(Tokenizer *tokenizer);
bool tokenizer_load_next(Tokenizer *tokenizer, Status *status);

#endif

/* vi: set et ts=4 sw=4: */
