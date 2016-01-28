#ifndef UTILS_H__
#define UTILS_H__

char*  _strdup(const char *s);
char*  _strndup(const char *s, size_t size);
void   die(const char *format, ...);
bool   empty_string(gchar *data);
gchar* find_next(gchar *data, gunichar c);
bool   next_char_equals(gchar *data, gunichar c);
gchar* delete_char(gchar *data);

#ifndef strdup
#define strdup _strdup
#endif

#ifndef strndup
#define strndup _strndup
#endif

#endif

