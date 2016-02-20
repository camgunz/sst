#include <stdlib.h>
#include <string.h>

#include "str.h"

StringStatus string_init(String *s, char *data) {
    string_free(s);

    if (data) {
        size_t len = strlen(data);

        s->data = calloc(len + 1, sizeof(char));

        if (!s->data) {
            return STRING_DATA_MEMORY_EXHAUSTED;
        }

        s->len = len;
        s->alloc = len + 1;
    }

    return STRING_OK;
}

StringStatus string_new(String **s, char *data) {
    String *new_string = malloc(sizeof(String));

    if (!new_string) {
        return STRING_DATA_MEMORY_EXHAUSTED;
    }

    if (string_init(new_string, data) != STRING_OK) {
        free(new_string);
        return STRING_DATA_MEMORY_EXHAUSTED;
    }

    *s = new_string;

    return STRING_OK;
}

void string_free(String *s) {
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->len = 0;
    s->alloc = 0;
}

void string_clear(String *s) {
    s->len = 0;
    if (s->data) {
        *s->data = '\0';
    }
}

StringStatus string_append(String *s, char *data) {
    return string_append_len(s, data, strlen(data));
}

StringStatus string_append_len(String *s, char *data, size_t len) {
    if (s->len + len >= s->alloc) {
        s->alloc = s->len + len + 1;
        s->data = realloc(s->data, s->alloc);

        if (!s->data) {
            s->len = 0;
            s->alloc = 0;

            return STRING_DATA_MEMORY_EXHAUSTED;
        }
    }

    memcpy(s->data + s->len, data, len);
    *(s->data + s->alloc) = '\0';

    return STRING_OK;
}

StringStatus string_assign(String *s, char *data) {
    return string_assign_len(s, data, strlen(data));
}

StringStatus string_assign_len(String *s, char *data, size_t len) {
    if (len + 1 > s->alloc) {
        s->alloc = len + 1;
        s->data = realloc(s->data, s->alloc);

        if (!s->data) {
            s->len = 0;
            s->alloc = 0;

            return STRING_DATA_MEMORY_EXHAUSTED;
        }
    }

    memcpy(s->data, data, len);
    *(s->data + s->alloc) = '\0';

    return STRING_OK;
}

/* vi: set et ts=4 sw=4: */

