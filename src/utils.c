#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utf8proc.h>

#include "config.h"
#include "rune.h"

char* _strndup(const char *cs, size_t maxlen) {
    size_t requested_size = maxlen + 1;
    char *s = malloc(requested_size * sizeof(char));

    strncpy(s, cs, requested_size);

    return s;
}

char* _strdup(const char *cs) {
    return strndup(cs, strlen(cs));
}

char* chardup(rune r) {
    uint8_t buf[4] = {0};
    ssize_t bytes_written;

    if (!utf8proc_codepoint_valid(r)) {
        return NULL;
    }

    bytes_written = utf8proc_encode_char(r, &buf[0]);

    if (bytes_written < 1) {
        return NULL;
    }

    return strndup((const char *)&buf[0], bytes_written);
}

void die(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

/* vi: set et ts=4 sw=4: */

