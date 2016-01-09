#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "str.h"
#include "tokenizer.h"

void tokenizer_init(Tokenizer *tokenizer) {
    tokenizer->data = NULL;
    tokenizer->raw = NULL;
    string_clear(&tokenizer->token);
}

void tokenizer_clear(Tokenizer *tokenizer) {
    if (tokenizer->data) {
        g_string_free(tokenizer->data);
    }

    tokenizer_init(tokenizer);
}

void tokenizer_reset(Tokenizer *tokenizer) {
    tokenizer->raw = NULL;
    string_clear(&tokenizer->token);
}

void tokenizer_set_data(Tokenizer *tokenizer, GString *data) {
    tokenizer_clear(tokenizer);
    if (!g_utf8_validate(data->str, -1, NULL)) {
        die("Invalid UTF-8 data passed to tokenizer\n");
    }
    tokenizer->data = data;
}

static bool load_next(Tokenizer *tokenizer) {
    if (!tokenizer->raw) {
        if (!tokenizer->data) {
            die("No tokenizer data\n");
        }

        tokenizer->raw = tokenizer->data->str;
    }

    while (true) {
        if ((*tokenizer->raw) == '\0') {
            string_clear(&tokenizer->token);
            return false;
        }

        if (!g_unichar_isspace(g_utf8_get_char(tokenizer->raw))) {
            break;
        }

        tokenizer->raw = g_utf8_next_char(tokenizer->raw);
    }

    tokenizer->token.start = tokenizer->raw;

    while (true) {
        if (g_unichar_isspace(g_utf8_get_char(tokenizer->raw))) {
            break;
        }

        ptrdiff_t len = tokenizer->raw - tokenizer->token.start;

        tokenizer->token.length = (gsize)len + 1;

        tokenizer->raw = g_utf8_next_char(tokenizer->raw);

        if ((*tokenizer->raw) == '\0') {
            break;
        }
    }

    return true;
}

/*
 * I want the next code tag
 *   - include
 *   - expression
 *   - if
 *   - for
 *   - raw
 * I want the contents of this code tag
 *   - include
 *   - expression
 *   - conditional
 *   - iteration
 * I want the block this code tag refers to
 *   - Requires knowing the opening tag's type
 *     - if   -> elif
 *     - if   -> else
 *     - if   -> endif
 *     - elif -> else
 *     - elif -> endif
 *     - else -> endif
 *     - for  -> endfor
 *     - raw  -> endraw
 *
 */

bool tokenizer_load_next(Tokenizer *tokenizer) {
    while (tokenizer_load_next(tokenizer)) {
        if (!g_unichar_isspace(tokenizer->token)) {
            return true;
        }
    }

    return false;
}

/* vi: set et ts=4 sw=4: */

