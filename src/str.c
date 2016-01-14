#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include "str.h"

void string_clear(String *s) {
    s->data = NULL;
    s->len = 0;
}

bool string_empty(String *s) {
    if (!s->data) {
        return true;
    }

    if (s->len == 0) {
        return true;
    }

    return false;
}

bool string_equals(String *s, const gchar *cs) {
    gsize i = 0;

    while (true) {
        if (i == s->len) {
            if (cs[i] == '\0') {
                return true;
            }

            return false;
        }

        if (s->data[i] != cs[i]) {
            return false;
        }

        i++;
    }
}

bool string_starts_with(String *s, const gchar *cs) {
    gsize i = 0;

    while (true) {
        if (cs[i] == '\0') {
            return true;
        }

        if (i == s->len) {
            return false;
        }

        if (s->data[i] != cs[i]) {
            return false;
        }

        i++;
    }
}

bool string_first_char(String *s, gunichar *uc) {
    if (string_empty(s)) {
        return false;
    }

    *uc = g_utf8_get_char(s->data);
    return true;
}

static bool advance_to_next_char(String *s) {
    gchar *s2 = g_utf8_next_char(s->data);

    if (!s2) {
        string_clear(s);
        return true;
    }

    if (s2 < s->data) {
        return false;
    }

    s->len = s2 - s->data;

    if (s->len == 0) {
        string_clear(s);
    }
    else {
        s->data = s2;
    }

    return true;
}

bool string_pop_char(String *s, gunichar *uc) {
    if (string_empty(s)) {
        return false;
    }

    if (uc) {
        *uc = g_utf8_get_char(s->data);
    }

    return advance_to_next_char(s);
}

bool string_first_char_equals(String *s, gunichar uc) {
    if (string_empty(s)) {
        return false;
    }

    gunichar uc2 = g_utf8_get_char(s->data);

    return uc2 == uc;
}

bool string_pop_char_if_equals(String *s, gunichar uc) {
    if (!string_first_char_equals(s, uc)) {
        return false;
    }

    return advance_to_next_char(s);
}

bool string_pop_char_if_digit(String *s, gunichar *uc) {
    if (string_empty(s)) {
        return false;
    }

    gunichar uc2 = g_utf8_get_char(s->data);

    if (!g_unichar_isdigit(uc2)) {
        return false;
    }

    if (uc) {
        *uc = uc2;
    }

    return advance_to_next_char(s);
}

bool string_pop_char_if_alnum(String *s, gunichar *uc) {
    if (string_empty(s)) {
        return false;
    }

    gunichar uc2 = g_utf8_get_char(s->data);

    if (!g_unichar_isalnum(uc2)) {
        return false;
    }

    if (uc) {
        *uc = uc2;
    }

    return advance_to_next_char(s);
}

gchar* string_find(String *s, gunichar uc) {
    if (string_empty(s)) {
        return false;
    }

    return g_utf8_strchr(s->data, s->len, uc);
}

/* vi: set et ts=4 sw=4: */

