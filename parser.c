#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMARK_NO_SHORT_NAMES
#include <json.h>
#include <cmark.h>
#include <glib.h>

enum Symbols  {
    EQUALS,
    NOT_EQUALS,
    GREATER_THAN,
    GREATHER_THAN_OR_EQUALS,
    LESS_THAN,
    LESS_THAN_OR_EQUALS,
    AND,
    OR,
    NOT,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    REMAINDER,
    EXPONENT,
    OPEN_BRACE,
    CLOSE_BRACE,
    SYMBOL_MAX
};

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

enum Keywords {
    IF,
    FOR,
    INCLUDE,
    KEYWORD_MAX
}

const char *KeywordValues[KEYWORD_MAX] = {
    "if",
    "for",
    "include"
}

typedef struct {
    bool in_directive;
} Parser

/* vi: set et ts=4 sw=4: */

