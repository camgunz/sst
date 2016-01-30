#ifndef UTILS_H__
#define UTILS_H__

char*  chardup(gunichar uc);
void   die(const char *format, ...);
bool   empty_string(gchar *data);
gchar* find_next(gchar *data, gunichar c);
bool   next_char_equals(gchar *data, gunichar c);
gchar* delete_char(gchar *data);

#endif

