#ifndef UTILS_H__
#define UTILS_H__

char* _strdup(const char *cs);
char* bufdup(const char *buf, size_t size);
char* chardup(rune r);
void  die(const char *format, ...);
char* find_next(char *data, rune r);
bool  next_char_equals(char *data, rune r);
char* delete_char(char *data);

#ifndef strdup
#define strdup _strdup
#endif

#endif

/* vi: set et ts=4 sw=4: */

