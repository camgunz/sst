#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "tokenizer.h"

void tokenizer_init(Tokenizer *tokenizer) {
    tokenizer->data = NULL;
    tokenizer->raw = NULL;
    tokenizer->token = NULL;
}

void tokenizer_clear(Tokenizer *tokenizer) {
    if (tokenizer->data) {
        g_string_free(tokenizer->data);
    }

    tokenizer_init(tokenizer);
}

void tokenizer_set_data(Tokenizer *tokenizer, GString *data) {
    tokenizer_clear(tokenizer);
    if (!g_utf8_validate(data->str, -1, NULL)) {
        die("Invalid UTF-8 data passed to tokenizer\n");
    }
    tokenizer->data = data;
}

bool tokenizer_load_next(Tokenizer *tokenizer) {
    if (!tokenizer->raw) {
        if (!tokenizer->data) {
            die("No tokenizer data\n");
        }

        tokenizer->raw = tokenizer->data->str;
    }
    else {
        tokenizer->raw = g_utf8_next_char(tokenizer->raw);
    }

    if (!tokenizer->raw) {
        return false;
    }

    tokenizer->token = g_utf8_get_char(tokenizer->raw);
    return true;
}

/* vi: set et ts=4 sw=4: */

