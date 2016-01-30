#ifndef STR_H__
#define STR_H__

typedef struct {
    gchar *data;
    gsize  len;
} String;

void   string_assign(String *s, gchar *gs);
void   string_clear(String *s);
bool   string_empty(String *s);
bool   string_equals(String *s, const gchar *cs);
bool   string_starts_with(String *s, const gchar *cs);
bool   string_first_char(String *s, gunichar *uc);
bool   string_advance_char(String *s);
bool   string_pop_char(String *s, gunichar *uc);
bool   string_first_char_equals(String *s, gunichar uc);
bool   string_pop_char_if_equals(String *s, gunichar uc);
bool   string_pop_char_if_digit(String *s, gunichar *uc);
bool   string_pop_char_if_alnum(String *s, gunichar *uc);
bool   string_pop_char_if_identifier(String *s, gunichar *uc);
gchar* string_find(String *s, gunichar uc);
void   string_copy(String *dst, String *src);
char*  string_to_c_string(String *s);

#endif

