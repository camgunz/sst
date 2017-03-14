#include "lang.h"

OperatorInformation OperatorInfo[OP_MAX] = {
    {"(",  OP_ASSOC_NONE,  0, 0},
    {")",  OP_ASSOC_NONE,  0, 0},
    {"||", OP_ASSOC_LEFT,  1, 2},
    {"&&", OP_ASSOC_LEFT,  1, 2},
    {"!",  OP_ASSOC_RIGHT, 2, 1},
    {"<",  OP_ASSOC_LEFT,  3, 1},
    {"<=", OP_ASSOC_LEFT,  3, 2},
    {">",  OP_ASSOC_LEFT,  3, 2},
    {">=", OP_ASSOC_LEFT,  3, 2},
    {"!=", OP_ASSOC_LEFT,  3, 2},
    {"==", OP_ASSOC_LEFT,  3, 2},
    {"+",  OP_ASSOC_LEFT,  4, 2},
    {"-",  OP_ASSOC_LEFT,  4, 2},
    {"*",  OP_ASSOC_LEFT,  5, 2},
    {"/",  OP_ASSOC_LEFT,  5, 2},
    {"%",  OP_ASSOC_LEFT,  5, 2},
    {"+",  OP_ASSOC_RIGHT, 6, 1},
    {"-",  OP_ASSOC_RIGHT, 6, 1},
    {"^",  OP_ASSOC_RIGHT,  7, 2},
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include", "if", "else", "endif", "for", "in", "break", "continue",
    "endfor", "raw", "endraw"
};

const char *SymbolValues[SYMBOL_MAX] = {
    "(", ")", "!", "+", "-", "*", "/", "%", "^", ",", "[", "]", ".", "&&",
    "||", "==", "!=", "<=", "<", ">=", ">"
};

/* vi: set et ts=4 sw=4: */
