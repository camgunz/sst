#include "lang.h"

OperatorInformation OperatorInfo[OP_MAX] = {
    {OP_ASSOC_LEFT,  1},
    {OP_ASSOC_LEFT,  1},
    {OP_ASSOC_RIGHT, 2},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  3},
    {OP_ASSOC_LEFT,  4},
    {OP_ASSOC_LEFT,  4},
    {OP_ASSOC_LEFT,  5},
    {OP_ASSOC_LEFT,  5},
    {OP_ASSOC_LEFT,  5},
    {OP_ASSOC_RIGHT, 6},
    {OP_ASSOC_RIGHT, 6},
    {OP_ASSOC_LEFT,  7},
    {OP_ASSOC_NONE,  8},
    {OP_ASSOC_NONE,  8},
};

const char *KeywordValues[KEYWORD_MAX] = {
    "include", "if", "elif", "else", "endif", "for", "in", "break", "endfor",
    "raw", "endraw"
};

const char *SymbolValues[SYMBOL_MAX] = {
    "(", ")", "!", "+", "-", "*", "/", "%", "^", ",", "[", "]", ".", "&&",
    "||", "==", "!=", "<=", "<", ">=", ">"
};

/* vi: set et ts=4 sw=4: */
