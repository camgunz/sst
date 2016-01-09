#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMARK_NO_SHORT_NAMES
#include <json.h>
#include <cmark.h>
#include <glib.h>

const char *SymbolValues[SYMBOL_MAX] = {
    "==",
    "!=",
    "!",
    ">",
    ">=",
    "<",
    "<=",
    "!!",
    "&&",
    "+",
    "-",
    "*",
    "/",
    "%",
    "^",
    "{{",
    "}}"
};

const char *KeywordValues[KEYWORD_MAX] = {
    "if",
    "for",
    "include"
}

typedef struct {
    bool in_directive;
} Parser

/* vi: set et ts=4 sw=4: */

