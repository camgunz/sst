#ifndef LEXER_H__
#define LEXER_H__

typedef enum {
    LEXER_OK                    = SST_OK,
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
    SSlice         data;
    SSlice         tag;
    bool           in_code;
    TokenQueue     tokens;
    mpd_context_t  mpd_ctx;
} Lexer;

void        lexer_init(Lexer *lexer);
void        lexer_clear(Lexer *lexer);
void        lexer_set_data(Lexer *lexer, SSlice *data);
LexerStatus lexer_load_next(Lexer *lexer);
Token*      lexer_get_previous_token(Lexer *lexer);
Token*      lexer_get_current_token(Lexer *lexer);

#endif

/* vi: set et ts=4 sw=4: */

