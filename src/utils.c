#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

char* chardup(gunichar uc) {
    return g_strdup_printf("%c", uc);
}

void die(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

bool empty_string(gchar *data) {
    if (!data) {
        return true;
    }

    if ((*data) == '\0') {
        return true;
    }

    return false;
}

gchar* find_next(gchar *data, gunichar c) {
    for (gchar *s = data; !empty_string(s); s = g_utf8_next_char(s)) {
        if (g_utf8_get_char(s) == c) {
            return s;
        }
    }

    return NULL;
}

bool next_char_equals(gchar *data, gunichar c) {
    gchar *next = g_utf8_next_char(data);

    if (empty_string(next)) {
        return false;
    }

    if (g_utf8_get_char(next) != c) {
        return false;
    }

    return true;
}

gchar* delete_char(gchar *data) {
    gchar *next = g_utf8_next_char(data);

    if (empty_string(next)) {
        return NULL;
    }

    return next;
}

