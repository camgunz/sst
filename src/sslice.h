#ifndef SSLICE_H__
#define SSLICE_H__

typedef enum {
    SSLICE_OK,
    SSLICE_MEMORY_EXHAUSTED,
    SSLICE_OVERFLOW,
    SSLICE_INVALID_UTF8,
    SSLICE_NOT_ASSIGNED,
    SSLICE_INVALID_OPTS,
    SSLICE_END,
    SSLICE_ERROR_UNKNOWN
} SSliceStatus;

typedef struct {
    char   *data;
    size_t  len;
} SSlice;

SSliceStatus sslice_base_assign(SSlice *s, char *cs, bool validate);
void         sslice_clear(SSlice *s);
bool         sslice_empty(SSlice *s);
bool         sslice_equals(SSlice *s, const char *cs);
bool         sslice_starts_with(SSlice *s, const char *cs);
SSliceStatus sslice_get_first_rune(SSlice *s, rune *r);
SSliceStatus sslice_advance_rune(SSlice *s);
SSliceStatus sslice_advance_runes(SSlice *s, size_t rune_count);
SSliceStatus sslice_advance_bytes(SSlice *s, size_t byte_count);
SSliceStatus sslice_pop_rune(SSlice *s, rune *r);
bool         sslice_first_rune_equals(SSlice *s, rune r);
bool         sslice_pop_rune_if_equals(SSlice *s, rune r);
bool         sslice_pop_rune_if_digit(SSlice *s, rune *r);
bool         sslice_pop_rune_if_alnum(SSlice *s, rune *r);
bool         sslice_pop_rune_if_identifier(SSlice *s, rune *r);
SSliceStatus sslice_seek_to(SSlice *s, rune r);
SSliceStatus sslice_seek_to_string(SSlice *s, const char *cs);
SSliceStatus sslice_seek_past_subslice(SSlice *s, SSlice *subslice);
SSliceStatus sslice_truncate_rune(SSlice *s);
SSliceStatus sslice_truncate_runes(SSlice *s, size_t rune_count);
SSliceStatus sslice_truncate_at(SSlice *s, rune r);
SSliceStatus sslice_truncate_at_whitespace(SSlice *s);
SSliceStatus sslice_truncate_at_subslice(SSlice *s, SSlice *subslice);
void         sslice_shallow_copy(SSlice *dst, SSlice *src);
bool         sslice_deep_copy(SSlice *dst, SSlice *src);
char*        sslice_to_c_string(SSlice *s);

#define sslice_assign(s, cs) sslice_base_assign(s, cs, false)
#define sslice_assign_validate(s, cs) sslice_base_assign(s, cs, true)

#endif

/* vi: set et ts=4 sw=4: */

