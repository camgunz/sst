#ifndef STR_H__
#define STR_H__

typedef enum {
    STRING_OK,
    STRING_MEMORY_EXHAUSTED,
    STRING_OVERFLOW,
    STRING_INVALID_UTF8,
    STRING_NOT_ASSIGNED,
    STRING_INVALID_OPTS,
    STRING_END,
    STRING_ERROR_UNKNOWN
} StringStatus;

typedef struct {
    char   *data;
    size_t  len;
} String;

StringStatus string_base_assign(String *s, char *cs, bool validate);
void         string_clear(String *s);
bool         string_empty(String *s);
bool         string_equals(String *s, const char *cs);
bool         string_starts_with(String *s, const char *cs);
StringStatus string_get_first_rune(String *s, rune *r);
StringStatus string_advance_rune(String *s);
StringStatus string_pop_rune(String *s, rune *r);
bool         string_first_rune_equals(String *s, rune r);
bool         string_pop_rune_if_equals(String *s, rune r);
bool         string_pop_rune_if_digit(String *s, rune *r);
bool         string_pop_rune_if_alnum(String *s, rune *r);
bool         string_pop_rune_if_identifier(String *s, rune *r);
StringStatus string_seek_to(String *s, rune r);
void         string_shallow_copy(String *dst, String *src);
bool         string_deep_copy(String *dst, String *src);
char*        string_to_c_string(String *s);

#define string_assign(s, cs) string_base_assign(s, cs, false)
#define string_assign_validate(s, cs) string_base_assign(s, cs, true)

#endif

