#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <utf8proc.h>

#include "rune.h"
#include "utils.h"
#include "str.h"

static StringStatus encode_rune(String *s, rune r) {
    uint8_t buf[4] = {0};
    ssize_t bytes_written;

    if (s->len < 4) {
        return STRING_OVERFLOW;
    }

    if (!utf8proc_codepoint_valid(r)) {
        return STRING_INVALID_UTF8;
    }

    bytes_written = utf8proc_encode_rune(r, &buf[0]);

    if (bytes_written < 1) {
        return STRING_ERROR_UNKNOWN;
    }

    for (int i = 0; i < bytes_written; i++) {
        s->data[i] = buf[i];
    }
    s->len = bytes_written;

    return STRING_OK;
}

StringStatus string_base_assign(String *s, char *cs, bool validate) {
    if (validate) {
        String cursor
        int32_t cc;
        ssize_t bytes_read;

        cursor.data = cs;
        cursor.len = strlen(cs);

        while (true) {
            bytes_read = utf8proc_iterate(cursor.data, cursor.len, &cc);

            if (bytes_read < 1) {
                switch (bytes_read) {
                    case UTF8PROC_ERROR_NOMEM: {
                        return STRING_MEMORY_EXHAUSTED;
                    }
                    case UTF8PROC_ERROR_OVERFLOW: {
                        return STRING_OVERFLOW;
                    }
                    case UTF8PROC_ERROR_INVALIDUTF8: {
                        return STRING_INVALID_UTF8;
                    }
                    case UTF8PROC_ERROR_NOTASSIGNED: {
                        return STRING_NOT_ASSIGNED;
                    }
                    case UTF8PROC_ERROR_INVALIDOPTS: {
                        return STRING_INVALID_OPTS;
                    }
                    default: {
                        return STRING_ERROR_UNKNOWN;
                    }
                }
            }

            cursor.data += bytes_read;
            cursor.len -= bytes_read;
        }
    }

    s->data = cs;
    s->len = strlen(cs);

    return STRING_OK;
}

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

bool string_equals(String *s, const char *cs) {
    size_t i = 0;

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

bool string_starts_with(String *s, const char *cs) {
    size_t i = 0;

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

StringStatus string_get_first_rune(String *s, rune *r) {
    rune r2;
    ssize_t bytes_read;

    if (string_empty(s)) {
        return STRING_END;
    }

    bytes_read = utf8proc_iterate(s->data, s->len, &r2);

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM: {
                return STRING_MEMORY_EXHAUSTED;
            }
            case UTF8PROC_ERROR_OVERFLOW: {
                return STRING_OVERFLOW;
            }
            case UTF8PROC_ERROR_INVALIDUTF8: {
                return STRING_INVALID_UTF8;
            }
            case UTF8PROC_ERROR_NOTASSIGNED: {
                return STRING_NOT_ASSIGNED;
            }
            case UTF8PROC_ERROR_INVALIDOPTS: {
                return STRING_INVALID_OPTS;
            }
            default: {
                return STRING_ERROR_UNKNOWN;
            }
        }
    }

    *r = r2;

    return STRING_OK;
}

StringStatus string_advance_rune(String *s) {
    rune r;
    ssize_t bytes_read;

    if (string_empty(s)) {
        return STRING_END;
    }

    bytes_read = utf8proc_iterate(s->data, s->len, &r);

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM: {
                return STRING_MEMORY_EXHAUSTED;
            }
            case UTF8PROC_ERROR_OVERFLOW: {
                return STRING_OVERFLOW;
            }
            case UTF8PROC_ERROR_INVALIDUTF8: {
                return STRING_INVALID_UTF8;
            }
            case UTF8PROC_ERROR_NOTASSIGNED: {
                return STRING_NOT_ASSIGNED;
            }
            case UTF8PROC_ERROR_INVALIDOPTS: {
                return STRING_INVALID_OPTS;
            }
            default: {
                return STRING_ERROR_UNKNOWN;
            }
        }
    }

    if (bytes_read < s->len) {
        s->data += bytes_read;
        s->len -= bytes_read;
    }
    else {
        string_clear(s);
    }

    return STRING_OK;
}

StringStatus string_pop_rune(String *s, rune *r) {
    rune r2;
    ssize_t bytes_read;

    if (string_empty(s)) {
        return STRING_END;
    }

    bytes_read = utf8proc_iterate(s->data, s->len, &r2);

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM: {
                return STRING_MEMORY_EXHAUSTED;
            }
            case UTF8PROC_ERROR_OVERFLOW: {
                return STRING_OVERFLOW;
            }
            case UTF8PROC_ERROR_INVALIDUTF8: {
                return STRING_INVALID_UTF8;
            }
            case UTF8PROC_ERROR_NOTASSIGNED: {
                return STRING_NOT_ASSIGNED;
            }
            case UTF8PROC_ERROR_INVALIDOPTS: {
                return STRING_INVALID_OPTS;
            }
            default: {
                return STRING_ERROR_UNKNOWN;
            }
        }
    }

    s->data += bytes_read;
    s->len -= bytes_read;

    *r = r2;

    return STRING_OK;
}

bool string_first_rune_equals(String *s, rune r) {
    rune r2;
    StringStatus res;

    res = string_get_first_rune(s, &r2);

    if (res != STRING_OK) {
        return false;
    }

    return r2 == r;
}

bool string_pop_rune_if_equals(String *s, rune r) {
    if (!string_first_rune_equals(s, r)) {
        return false;
    }

    return string_advance_rune(s) == STRING_OK;
}

bool string_pop_rune_if_digit(String *s, rune *r) {
    rune r2;
    StringStatus res;

    res = string_get_first_rune(s, &r2);

    if (res != STRING_OK) {
        return false;
    }

    if (!rune_is_digit(r2)) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return string_advance_rune(s) == STRING_OK;
}

bool string_pop_rune_if_alnum(String *s, rune *r) {
    rune r2;
    StringStatus res;

    res = string_get_first_rune(s, &r2);

    if (res != STRING_OK) {
        return false;
    }

    if (!(rune_is_alpha(r2) || rune_is_digit(r2))) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return string_advance_rune(s) == STRING_OK;
}

bool string_pop_rune_if_identifier(String *s, rune *r) {
    rune r2;
    StringStatus res;

    res = string_get_first_rune(s, &r2);

    if (res != STRING_OK) {
        return false;
    }

    if (!(rune_is_alpha(r2) || rune_is_digit(r2) || r2 == '.' || r2 == '_')) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return string_advance_rune(s);
}

StringStatus string_pop_if_number(String *s, mpd_context_t *ctx, mpd_t **num

StringStatus string_advance_bytes(String *s, size_t byte_count) {
    if (byte_count > s->len) {
        return STRING_OVERFLOW;
    }

    if (byte_count == s->len) {
        string_clear(s);
        return STRING_OK;
    }

    s->data += byte_count;
    s->len -= byte_count;
}

StringStatus string_seek_to(String *s, rune r) {
    String cursor;
    StringStatus res;
    rune r2;

    string_shallow_copy(&cursor, s);

    res = string_pop_rune(&cursor, &r2);

    while (res == STRING_OK) {
        if (r2 == r) {
            string_shallow_copy(s, &cursor);
            break;
        }
    }

    return res;
}

StringStatus string_truncate_at(String *s, rune r) {
    String cursor;

    string_shallow_copy(&cursor, s);

    while (true) {
        rune r2;
        StringStatus res = string_pop_rune(&cursor, &r2);

        if (res != STRING_OK) {
            return res;
        }

        if (r2 == r) {
            s->len = (cursor.data - s->data) + 1;
            return STRING_OK;
        }
    }
}

StringStatus string_truncate_at_whitespace(String *s) {
    String cursor;

    string_shallow_copy(&cursor, s);

    while (true) {
        rune r;
        StringStatus res = string_pop_rune(&cursor, &r);

        if (res != STRING_OK) {
            return res;
        }

        if (rune_is_whitespace(r)) {
            s->len = (cursor.data - s->data) + 1;
            return STRING_OK;
        }
    }
}

StringStatus string_truncate_runes(String *s, size_t rune_count) {
    String cursor;
    rune r;
    StringStatus res;

    string_shallow_copy(&cursor, s);

    cursor.data += (cursor.len - 1);
    cursor.len = 1;

    while (true) {
        res = string_first_rune(&cursor, &r);

        if (res == STRING_OK) {
            if (rune_count == 1) {
                return STRING_OK;
            }

            rune_count--;
        }
        else if (res == STRING_INVALID_UTF8) {
            cursor.data--;
            cursor.len++;

            if (cursor.data == s->data) {
                return STRING_END;
            }
        }
    }
}

bool string_deep_copy(String *dst, String *src) {
    char *data = strndup(src->data, src->len);

    if (!data) {
        return false;
    }

    dst->data = data;
    dst->len  = src->len;

    return true;
}

void string_shallow_copy(String *dst, String *src) {
    dst->data = src->data;
    dst->len  = src->len;
}

bool string_deep_copy(String *dst, String *src) {
    char *data = strndup(src->data, src->len);

    if (!data) {
        return false;
    }

    dst->data = data;
    dst->len  = src->len;

    return true;
}

char* string_to_c_string(String *s) {
    return strndup(s->data, s->len);
}

/* vi: set et ts=4 sw=4: */

