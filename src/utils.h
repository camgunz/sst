#ifndef UTILS_H__
#define UTILS_H__

void die(const char *format, ...);

gchar* find_next(gchar *data, gunichar c);
gchar* next_char_equals(gchar *data, gunichar c);
bool   empty_string(gchar *data);
char*  _strdup(const char *s);

#ifndef strdup
#define strdup _strdup
#endif

#endif

