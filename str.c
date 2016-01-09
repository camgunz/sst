#include <stdbool.h>
#include <string.h>

#include <glib.h>

#include "str.h"

void string_clear(String *s) {
    s->data = NULL;
    s->len = 0;
}

bool string_equals(String *s, const char *cs) {
    gsize i = 0;
    gsize ssize = s->len;

    while (true) {
        const char csc = cs[i];

        if (i == slen) {
            if (csc == '\0') {
                return true;
            }

            return false;
        }

        if (s[i] != csc) {
            return false;
        }
    }
}

