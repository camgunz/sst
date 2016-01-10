#ifndef LEXER_H__
#define LEXER_H__

typedef struct {
    gchar  *data;
    String  section;
    bool    section_is_code;
} Lexer;

void lexer_clear(Lexer *lexer);
void lexer_init(Lexer *lexer, gchar *data);
bool lexer_load_next(Lexer *lexer);

#endif

