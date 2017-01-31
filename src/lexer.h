#ifndef LEXER_H__
#define LEXER_H__

enum {
    LEXER_INVALID_STATUS,
    LEXER_UNKNOWN_TOKEN,
    LEXER_UNEXPECTED_WHITESPACE,
    LEXER_UNEXPECTED_SYMBOL,
    LEXER_UNEXPECTED_TOKEN,
    LEXER_UNEXPECTED_NUMBER,
    LEXER_EXPECTED_NUMBER,
    LEXER_UNEXPECTED_CLOSE_BRACKET,
};

typedef enum {
    CODE_TOKEN_UNKNOWN,
    CODE_TOKEN_TEXT,
    CODE_TOKEN_NUMBER,
    CODE_TOKEN_STRING,
    CODE_TOKEN_KEYWORD,
    CODE_TOKEN_LOOKUP,
    CODE_TOKEN_FUNCTION_START,
    CODE_TOKEN_FUNCTION_END,
    CODE_TOKEN_INDEX_START,
    CODE_TOKEN_INDEX_END,
    CODE_TOKEN_ARRAY_START,
    CODE_TOKEN_ARRAY_END,
    CODE_TOKEN_OPERATOR,
} CodeTokenType;

typedef struct {
    CodeTokenType type;
    const char *location;
    union {
        SSlice text;
        SSlice number;
        SSlice string;
        Keyword keyword;
        SSlice lookup;
        SSlice function;
        SSlice index;
        Operator op;
    } as;
} CodeToken;

typedef enum {
    LEXER_STATE_FUNCTION = 1,
    LEXER_STATE_PAREN = 2,
    LEXER_STATE_INDEX = 4,
    LEXER_STATE_ARRAY = 8,
} LexerState;

typedef struct {
    Tokenizer tokenizer;
    CodeToken code_token;
    bool already_loaded_next;
    Array states;
} Lexer;

/*
 * The lexer takes basic tokens and processes them into code tokens.  Mainly,
 * this is useful for:
 *   - function calls {{ say("sup", 4) }}
 *   - indexing {{ people[28] }}
 *   - lookups {{ us.senators.amy_klobuchar }}
 *   - operators {{ 14 - -3 * -14 / +(28.2892 * -.07630) }}
 *
 * Notably, a code token will not be whitespace, a symbol, or an identifier.
 * These are all lexed into the above.
 */

bool lexer_init(Lexer *lexer, SSlice *data, Status *status);
void lexer_clear(Lexer *lexer);
void lexer_set_data(Lexer *lexer, SSlice *data);
bool lexer_load_next(Lexer *lexer, Status *status);
bool lexer_get_previous_token(Lexer *lexer, Token **token, Status *status);
bool lexer_get_current_token(Lexer *lexer, Token **token, Status *status);

#endif

/* vi: set et ts=4 sw=4: */
