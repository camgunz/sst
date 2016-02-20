#ifndef STR_H__
#define STR_H__

typedef struct {
    char *data;
    size_t len;
    size_t alloc;
} String;

typedef enum {
    STRING_OK,
    STRING_DATA_MEMORY_EXHAUSTED
} StringStatus;

StringStatus string_init(String *s, char *data);
StringStatus string_new(String **s, char *data);
void         string_clear(String *s);
void         string_free(String *s);
StringStatus string_append(String *s, char *data);
StringStatus string_append_len(String *s, char *data, size_t len);
StringStatus string_assign(String *s, char *data);
StringStatus string_assign_len(String *s, char *data, size_t len);

#endif

/* vi: set et ts=4 sw=4: */

