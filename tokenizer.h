#ifndef TOKENIZER_H__
#define TOKENIZER_H__

typedef struct {
    GString  *data;
    gchar    *raw;
    String    token;
} Tokenizer;

void tokenizer_init(Tokenizer *tokenizer);
void tokenizer_clear(Tokenizer *tokenizer);
void tokenizer_set_data(Tokenizer *tokenizer, GString *data);
bool tokenizer_load_next(Tokenizer *tokenizer);

#endif

