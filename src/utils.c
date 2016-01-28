#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

char* _strdup(const char *s) {
    char *s2 = calloc(strlen(s) + 1, sizeof(char));

    strcpy(s2, s);

    return s2;
}

char* _strndup(const char *s, size_t size) {
    char *s2 = calloc(size + 1, sizeof(char));

    strncpy(s2, s, size);

    return s2;
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

