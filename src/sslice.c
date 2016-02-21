#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <stdint.h>
#include <utf8proc.h>

#include "rune.h"
#include "utils.h"
#include "sslice.h"

static SSliceStatus encode_rune(SSlice *s, rune r) {
    uint8_t buf[4] = {0};
    ssize_t bytes_written;

    if (s->len < 4) {
        return SSLICE_OVERFLOW;
    }

    if (!utf8proc_codepoint_valid(r)) {
        return SSLICE_INVALID_UTF8;
    }

    bytes_written = utf8proc_encode_char(r, &buf[0]);

    if (bytes_written < 1) {
        return SSLICE_ERROR_UNKNOWN;
    }

    for (int i = 0; i < bytes_written; i++) {
        s->data[i] = buf[i];
    }
    s->len = bytes_written;

    return SSLICE_OK;
}

SSliceStatus sslice_base_assign(SSlice *s, char *cs, bool validate) {
    if (validate) {
        SSlice cursor;
        int32_t cc;
        ssize_t bytes_read;

        cursor.data = cs;
        cursor.len = strlen(cs);

        while (cursor.len > 0) {
            bytes_read = utf8proc_iterate(cursor.data, sizeof(int32_t), &cc);

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
    s->len = strlen(cs);

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

    bytes_read = utf8proc_iterate(s->data, sizeof(rune), &r2);

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

    bytes_read = utf8proc_iterate(s->data, sizeof(rune), &r);

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
    }
    else {
        sslice_clear(s);
    }

    return SSLICE_OK;
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

SSliceStatus sslice_advance_bytes(SSlice *s, size_t byte_count) {
    if (byte_count >= s->len) {
        sslice_clear(s);
    }
    else {
        s->data += byte_count;
        s->len -= byte_count;
    }

    return SSLICE_OK;
}

SSliceStatus sslice_pop_rune(SSlice *s, rune *r) {
    rune r2;
    ssize_t bytes_read;

    if (sslice_empty(s)) {
        return SSLICE_END;
    }

    bytes_read = utf8proc_iterate(s->data, sizeof(rune), &r2);

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
    return sslice_advance_bytes(s, subslice->len + 1);
}

SSliceStatus sslice_truncate_rune(SSlice *s) {
    char *cursor;
    size_t size = 0;
    ssize_t bytes_read;
    int32_t codepoint;

    cursor = s->data + s->len;

    while (true) {
        cursor--;
        size++;
        
        if (size == s->len) {
            return SSLICE_END;
        }

        bytes_read = utf8proc_iterate(cursor, size, &codepoint);

        if (bytes_read > 0) {
            break;
        }

        switch (bytes_read) {
            case UTF8PROC_ERROR_NOMEM:
                return SSLICE_MEMORY_EXHAUSTED;
            case UTF8PROC_ERROR_OVERFLOW:
                return SSLICE_OVERFLOW;
            case UTF8PROC_ERROR_NOTASSIGNED:
                return SSLICE_NOT_ASSIGNED;
            case UTF8PROC_ERROR_INVALIDOPTS:
                return SSLICE_INVALID_OPTS;
            case UTF8PROC_ERROR_INVALIDUTF8:
            default:
                break;
        }
    }

    s->len -= size;

    if (s->len == 0) {
        sslice_clear(s);
    }

    return SSLICE_OK;
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

    sslice_shallow_copy(&cursor, s);

    while (true) {
        rune r2;
        SSliceStatus res = sslice_pop_rune(&cursor, &r2);

        if (res != SSLICE_OK) {
            return res;
        }

        if (r2 == r) {
            s->len = (cursor.data - s->data) - 2;
            return SSLICE_OK;
        }
    }
}

SSliceStatus sslice_truncate_at_whitespace(SSlice *s) {
    SSlice cursor;

    sslice_shallow_copy(&cursor, s);

    while (true) {
        rune r;
        SSliceStatus res = sslice_pop_rune(&cursor, &r);

        if (res != SSLICE_OK) {
            return res;
        }

        if (rune_is_whitespace(r)) {
            s->len = (cursor.data - s->data) - 2;
            return SSLICE_OK;
        }
    }
}

SSliceStatus sslice_truncate_at_subslice(SSlice *s, SSlice *subslice) {
    size_t new_len = (subslice->data - s->data) - 1;

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
    char *data = strndup(src->data, src->len);

    if (!data) {
        return false;
    }

    dst->data = data;
    dst->len  = src->len;

    return true;
}

char* sslice_to_c_string(SSlice *s) {
    return strndup(s->data, s->len);
}

/* vi: set et ts=4 sw=4: */

