#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <utf8proc.h>

#include "rune.h"
#include "utils.h"
#include "sslice.h"

void sslice_print(SSlice *s) {
    char *as_string = sslice_to_c_string(s);

    printf("[%s] (%zu)\n", as_string, s->len);

    free(as_string);
}

SSliceStatus sslice_base_assign(SSlice *s, char *cs, bool validate) {
    size_t slen = strlen(cs);

    if (validate) {
        SSlice cursor;
        int32_t cc;
        ssize_t bytes_read;

        cursor.data = cs;
        cursor.len = slen;

        while (cursor.len > 0) {
            bytes_read = utf8proc_iterate(
                (const unsigned char *)cursor.data, cursor.len, &cc
            );

            if (bytes_read < 1) {
                switch (bytes_read) {
                    case UTF8PROC_ERROR_NOMEM:
                        return SSLICE_MEMORY_EXHAUSTED;
                    case UTF8PROC_ERROR_OVERFLOW:
                        return SSLICE_OVERFLOW;
                    case UTF8PROC_ERROR_INVALIDUTF8:
                        return SSLICE_INVALID_UTF8;
                    case UTF8PROC_ERROR_NOTASSIGNED:
                        return SSLICE_NOT_ASSIGNED;
                    case UTF8PROC_ERROR_INVALIDOPTS:
                        return SSLICE_INVALID_OPTS;
                    default:
                        return SSLICE_ERROR_UNKNOWN;
                }
            }

            cursor.data += bytes_read;
            cursor.len -= bytes_read;
        }
    }

    s->data = cs;
    s->len = slen;

    return SSLICE_OK;
}

void sslice_clear(SSlice *s) {
    s->data = NULL;
    s->len = 0;
}

bool sslice_empty(SSlice *s) {
    if (!s->data) {
        return true;
    }

    if (s->len == 0) {
        return true;
    }

    return false;
}

bool sslice_equals(SSlice *s, const char *cs) {
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

bool sslice_starts_with(SSlice *s, const char *cs) {
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

SSliceStatus sslice_get_first_rune(SSlice *s, rune *r) {
    rune r2;
    ssize_t bytes_read;

    if (sslice_empty(s)) {
        return SSLICE_END;
    }

    bytes_read = utf8proc_iterate(
        (const unsigned char *)s->data, s->len, &r2
    );

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM:
                return SSLICE_MEMORY_EXHAUSTED;
            case UTF8PROC_ERROR_OVERFLOW:
                return SSLICE_OVERFLOW;
            case UTF8PROC_ERROR_INVALIDUTF8:
                return SSLICE_INVALID_UTF8;
            case UTF8PROC_ERROR_NOTASSIGNED:
                return SSLICE_NOT_ASSIGNED;
            case UTF8PROC_ERROR_INVALIDOPTS:
                return SSLICE_INVALID_OPTS;
            default:
                return SSLICE_ERROR_UNKNOWN;
        }
    }

    *r = r2;

    return SSLICE_OK;
}

SSliceStatus sslice_advance_rune(SSlice *s) {
    rune r;
    ssize_t bytes_read;

    if (sslice_empty(s)) {
        return SSLICE_END;
    }

    bytes_read = utf8proc_iterate(
        (const unsigned char *)s->data, s->len, &r
    );

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM:
                return SSLICE_MEMORY_EXHAUSTED;
            case UTF8PROC_ERROR_OVERFLOW:
                return SSLICE_OVERFLOW;
            case UTF8PROC_ERROR_INVALIDUTF8:
                return SSLICE_INVALID_UTF8;
            case UTF8PROC_ERROR_NOTASSIGNED:
                return SSLICE_NOT_ASSIGNED;
            case UTF8PROC_ERROR_INVALIDOPTS:
                return SSLICE_INVALID_OPTS;
            default:
                return SSLICE_ERROR_UNKNOWN;
        }
    }

    if (bytes_read < (ssize_t)s->len) {
        s->data += bytes_read;
        s->len -= bytes_read;
        return SSLICE_OK;
    }

    sslice_clear(s);

    return SSLICE_END;
}

SSliceStatus sslice_advance_runes(SSlice *s, size_t rune_count) {
    SSliceStatus res;

    for (size_t i = 0; i < rune_count; i++) {
        res = sslice_advance_rune(s);

        if (res != SSLICE_OK) {
            return res;
        }
    }

    return SSLICE_OK;
}

SSliceStatus sslice_pop_rune(SSlice *s, rune *r) {
    rune r2;
    ssize_t bytes_read;

    if (sslice_empty(s)) {
        return SSLICE_END;
    }

    bytes_read = utf8proc_iterate(
        (const unsigned char *)s->data, s->len, &r2
    );

    if (bytes_read < 1) {
        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM:
                return SSLICE_MEMORY_EXHAUSTED;
            case UTF8PROC_ERROR_OVERFLOW:
                return SSLICE_OVERFLOW;
            case UTF8PROC_ERROR_INVALIDUTF8:
                return SSLICE_INVALID_UTF8;
            case UTF8PROC_ERROR_NOTASSIGNED:
                return SSLICE_NOT_ASSIGNED;
            case UTF8PROC_ERROR_INVALIDOPTS:
                return SSLICE_INVALID_OPTS;
            default:
                return SSLICE_ERROR_UNKNOWN;
        }
    }

    s->data += bytes_read;
    s->len -= bytes_read;

    if (r) {
        *r = r2;
    }

    return SSLICE_OK;
}

bool sslice_first_rune_equals(SSlice *s, rune r) {
    rune r2;
    SSliceStatus res;

    res = sslice_get_first_rune(s, &r2);

    if (res != SSLICE_OK) {
        return false;
    }

    return r2 == r;
}

bool sslice_pop_rune_if_equals(SSlice *s, rune r) {
    if (!sslice_first_rune_equals(s, r)) {
        return false;
    }

    return sslice_advance_rune(s) == SSLICE_OK;
}

bool sslice_pop_rune_if_digit(SSlice *s, rune *r) {
    rune r2;
    SSliceStatus res;

    res = sslice_get_first_rune(s, &r2);

    if (res != SSLICE_OK) {
        return false;
    }

    if (!rune_is_digit(r2)) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return sslice_advance_rune(s) == SSLICE_OK;
}

bool sslice_pop_rune_if_alnum(SSlice *s, rune *r) {
    rune r2;
    SSliceStatus res;

    res = sslice_get_first_rune(s, &r2);

    if (res != SSLICE_OK) {
        return false;
    }

    if (!(rune_is_alpha(r2) || rune_is_digit(r2))) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return sslice_advance_rune(s) == SSLICE_OK;
}

bool sslice_pop_rune_if_identifier(SSlice *s, rune *r) {
    rune r2;
    SSliceStatus res;

    res = sslice_get_first_rune(s, &r2);

    if (res != SSLICE_OK) {
        return false;
    }

    if (!(rune_is_alpha(r2) || rune_is_digit(r2) || r2 == '.' || r2 == '_')) {
        return false;
    }

    if (r) {
        *r = r2;
    }

    return sslice_advance_rune(s);
}

SSliceStatus sslice_seek_to(SSlice *s, rune r) {
    SSlice cursor;
    SSliceStatus res;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        if (sslice_first_rune_equals(&cursor, r)) {
            break;
        }

        res = sslice_advance_rune(&cursor);

        if (res != SSLICE_OK) {
            break;
        }
    }

    if (res == SSLICE_OK) {
        sslice_shallow_copy(s, &cursor);
    }

    return res;
}

SSliceStatus sslice_seek_to_string(SSlice *s, const char *cs) {
    SSlice cursor;
    SSliceStatus res;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        if (sslice_starts_with(&cursor, cs)) {
            break;
        }

        res = sslice_advance_rune(&cursor);

        if (res != SSLICE_OK) {
            break;
        }
    }

    if (res == SSLICE_OK) {
        sslice_shallow_copy(s, &cursor);
    }

    return res;
}

SSliceStatus sslice_seek_past_subslice(SSlice *s, SSlice *subslice) {
    size_t byte_count = subslice->len + 1;

    if (byte_count >= s->len) {
        sslice_clear(s);
    }
    else {
        s->data += byte_count;
        s->len -= byte_count;
    }

    return SSLICE_OK;
}

void sslice_print_runes(SSlice *s) {
    SSlice copy;

    sslice_shallow_copy(&copy, s);

    while (true) {
        SSliceStatus sstatus;
        rune r = 0;

        sstatus = sslice_pop_rune(s, &r);

        if (sstatus != SSLICE_OK) {
            puts("");
        }

        printf("%c ", r);
    }
    puts("");
}

SSliceStatus sslice_truncate_rune(SSlice *s) {
    SSlice       copy;
    SSlice       saved;
    SSliceStatus sstatus;

    sslice_shallow_copy(&copy, s);

    while (true) {
        sslice_shallow_copy(&saved, &copy);
        sstatus = sslice_advance_rune(&copy);

        if (sstatus == SSLICE_END) {
            return sslice_truncate_at_subslice(s, &saved);
        }

        if (sstatus != SSLICE_OK) {
            break;
        }
    }

    return sstatus;
}

SSliceStatus sslice_truncate_runes(SSlice *s, size_t rune_count) {
    SSlice cursor;
    SSliceStatus res;

    sslice_shallow_copy(&cursor, s);

    for (size_t i = 0; i < rune_count; i++) {
        res = sslice_truncate_rune(s);

        if (res != SSLICE_OK) {
            return res;
        }
    }

    sslice_shallow_copy(s, &cursor);

    return SSLICE_OK;
}

SSliceStatus sslice_truncate_at(SSlice *s, rune r) {
    SSlice cursor;
    SSlice previous;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        rune r2;
        SSliceStatus res;

        sslice_shallow_copy(&previous, &cursor);

        res = sslice_pop_rune(&cursor, &r2);

        if (res != SSLICE_OK) {
            return res;
        }

        if (r2 == r) {
            s->len = previous.data - s->data;
            return SSLICE_OK;
        }
    }
}

SSliceStatus sslice_truncate_at_whitespace(SSlice *s) {
    SSlice cursor;
    SSlice previous;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        rune r;
        SSliceStatus res;

        sslice_shallow_copy(&previous, &cursor);

        res = sslice_pop_rune(&cursor, &r);

        if (res != SSLICE_OK) {
            return res;
        }

        if (rune_is_whitespace(r)) {
            s->len = previous.data - s->data;
            return SSLICE_OK;
        }
    }
}

SSliceStatus sslice_truncate_at_subslice(SSlice *s, SSlice *subslice) {
    size_t new_len;

    if (subslice->data <= s->data) {
        return SSLICE_END;
    }

    new_len = subslice->data - s->data;

    if (new_len >= s->len) {
        return SSLICE_END;
    }

    s->len = new_len;

    return SSLICE_OK;
}

void sslice_shallow_copy(SSlice *dst, SSlice *src) {
    dst->data = src->data;
    dst->len  = src->len;
}

bool sslice_deep_copy(SSlice *dst, SSlice *src) {
    char *data = bufdup(src->data, src->len);

    if (!data) {
        return false;
    }

    dst->data = data;
    dst->len  = src->len;

    return true;
}

char* sslice_to_c_string(SSlice *s) {
    size_t len = s->len + 1;
    char *str = calloc(len, sizeof(char));

    memcpy(str, s->data, s->len);

    return str;
}

/* vi: set et ts=4 sw=4: */

