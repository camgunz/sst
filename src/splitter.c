#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "utils.h"
#include "str.h"
#include "splitter.h"

static gchar* find_next_code_tag_start(gchar *data) {
    gchar *open = data;

    while (true) {
        open = find_next(open, '{');

        if (empty_string(open)) {
            return NULL;
        }

        if (next_char_equals(open, '{')) {
            return open;
        }

        open = g_utf8_next_char(open);
    }
}

static bool find_next_code_tag(gchar *data, String *tag) {
    gchar *open = find_next_code_tag_start(data);
    gchar *cursor = NULL;
    gunichar in_string = '\0';

    if (!open) {
        return false;
    }

    cursor = g_utf8_next_char(data);

    while (!empty_string(cursor)) {
        gunichar uc = g_utf8_get_char(cursor);

        switch (uc) {
            case '\'': {
                switch (in_string) {
                    case '\0': {
                        in_string = '\'';
                        break;
                    }
                    case '\'': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            case '`': {
                switch (in_string) {
                    case '\0': {
                        in_string = '`';
                        break;
                    }
                    case '`': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            case '"': {
                switch (in_string) {
                    case '\0': {
                        in_string = '"';
                        break;
                    }
                    case '"': {
                        in_string = '\0';
                        break;
                    }
                }
                break;
            }
            case '}': {
                if (in_string != '\0') {
                    break;
                }

                gchar *next = delete_char(cursor);

                gunichar uc = g_utf8_get_char(cursor);

                if (uc != '}') {
                    cursor = next;
                    break;
                }

                tag->data = open;
                tag->len = (next - open) + 1;

                return true;
            }
        }
        cursor = g_utf8_next_char(cursor);
    }

    return false;
}

void splitter_clear(Splitter *splitter) {
    splitter->data = NULL;
    splitter->section_is_code = false;
    string_clear(&splitter->section);
}

void splitter_init(Splitter *splitter, gchar *data) {
    if (!g_utf8_validate(data, -1, NULL)) {
        die("Invalid UTF-8 data passed to splitter\n");
    }

    splitter_clear(splitter);

    splitter->data = data;
    splitter->section_is_code = false;
}

bool splitter_load_next(Splitter *splitter) {
    gchar *s;

    if (splitter->section.data) {
        s = splitter->section.data + splitter->section.len;
    }
    else {
        s = splitter->data;
    }

    if (empty_string(s)) {
        return false;
    }

    gchar *code_tag_start = find_next_code_tag_start(s);

    if (!code_tag_start) {
        splitter->section_is_code = false;
        splitter->section.data = s;
        splitter->section.len = strlen(s);
    }
    else if (code_tag_start == s) {
        splitter->section_is_code = find_next_code_tag(
            code_tag_start, &splitter->section
        );

        if (!splitter->section_is_code) {
            splitter->section.data = code_tag_start;
            splitter->section.len = strlen(code_tag_start);
        }
    }
    else {
        splitter->section_is_code = false;
        splitter->section.data = s;
        splitter->section.len = code_tag_start - s;
    }

    return true;
}

/* vi: set et ts=4 sw=4: */

