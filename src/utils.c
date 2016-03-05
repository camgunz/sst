#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utf8proc.h>

#include "config.h"
#include "rune.h"

char* _strdup(const char *cs) {
    size_t len = strlen(cs) + 1;
    char *str = calloc(len, sizeof(char));

    if (!str) {
        return NULL;
    }

    memcpy(str, cs, len - 1);

    return str;
}

char* bufdup(const char *buf, size_t size) {
    char *newbuf = malloc(size);

    memcpy(newbuf, buf, size);

    return newbuf;
}

char* chardup(rune r) {
    uint8_t  buf[4] = {0};
    ssize_t  bytes_written;
    char    *s;

    if (!utf8proc_codepoint_valid(r)) {
        return NULL;
    }

    bytes_written = utf8proc_encode_char(r, &buf[0]);

    if (bytes_written < 1) {
        return NULL;
    }

    s = calloc(bytes_written + 1, sizeof(char));

    if (!s) {
        return NULL;
    }

    memcpy(s, &buf[0], bytes_written);

    return s;
}

void die(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

/* vi: set et ts=4 sw=4: */

