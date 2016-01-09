#ifndef STR_H__
#define STR_H__

typedef struct {
    gchar *data;
    gsize  len;
} String;

bool string_equals(String *s, const char *cs);

#endif

