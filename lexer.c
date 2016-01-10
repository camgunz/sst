#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "str.h"
#include "lexer.h"

static bool find_next_code_tag_start(gchar *data, String *tag) {
    gchar *open = data;

    while (true) {
        open = find_next(open, '{');

        if (!open) {
            return NULL;
        }

        if (next_char_equals(open, '{')) {
            return open;
        }
    }
}

static bool find_next_code_tag(gchar *data, String *tag) {
    gchar *open = data;
    gchar *cursor = NULL;
    gchar *close = NULL;
    gunichar in_string = '\0';

    while (true) {
        open = find_next(open, '{');

        if (!open) {
            return NULL;
        }

        cursor = next_char_equals(open, '{');

        if (cursor) {
            break;
        }
    }

    cursor = g_utf8_next_char(cursor);

    while (cursor && (*cursor) != '\0') {
        gunichar uc = g_utf8_get_char(cursor);

        switch (uc) {
            case '\'': {
                switch (in_string) {
                    case '\0':
                        in_string = '\'';
                        break;
                    case '\'':
                        in_string = '\0';
                        break;
                }
                break;
            }
            case '`': {
                switch (in_string) {
                    case '\0':
                        in_string = '`';
                        break;
                    case '`':
                        in_string = '\0';
                        break;
                }
                break;
            }
            case '"': {
                switch (in_string) {
                    case '\0':
                        in_string = '"';
                        break;
                    case '"':
                        in_string = '\0';
                        break;
                }
                break;
            }
            case '}': {
                if (in_string == '\0') {
                    close = next_char_equals(cursor, '}');

                    if (close) {
                        close = g_utf8_next_char(close);
                    }

                    tag->len = close - open;

                    return true;
                }
            }
        }
    }

    return false;
}

void lexer_clear(Lexer *lexer) {
    lexer->data = NULL;
    lexer->section_is_code = false;
    string_clear(&lexer->section);
}

void lexer_init(Lexer *lexer, gchar *data) {
    if (!g_utf8_validate(data, -1, NULL)) {
        die("Invalid UTF-8 data passed to lexer\n");
    }

    lexer_clear(lexer);

    lexer->data = data;
    lexer->section_is_code = false;
}

bool lexer_load_next(Lexer *lexer) {
    gchar *s;

    if (lexer->section.data) {
        s = lexer->section.data + lexer->section.len;
    }
    else {
        s = lexer->data;
    }

    if (!s) {
        return false;
    }

    if ((*s) == '\0') {
        return false;
    }

    gchar *code_tag_start = find_next_code_tag_start(s);

    if (!code_tag) {
        lexer->section_is_code = false;
        lexer->section.data = s;
        lexer->section.len = strlen(s);
    }
    else if (code_tag_start == s) {
        lexer->section_is_code = find_next_code_tag(
            code_tag_start, &lexer->section
        );

        if (!lexer->section_is_code) {
            lexer->section.data = code_tag_start;
            lexer->section.len = strlen(code_tag_start);
        }
    }
    else {
        lexer->section_is_code = false;
        lexer->section.data = s;
        lexer->section.len = code_tag_start - s;
    }

    return true;
}

/* vi: set et ts=4 sw=4: */

