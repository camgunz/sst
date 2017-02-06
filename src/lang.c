#include "lang.h"

OperatorInfo Operators[OP_MAX] = {
    {"&&", OP_CATEGORY_BINARY_BOOL, 1},
    {"||", OP_CATEGORY_BINARY_BOOL, 1},
    {"!",  OP_CATEGORY_UNARY_BOOL,  2},
    {"==", OP_CATEGORY_BINARY_BOOL, 3},
    {"!=", OP_CATEGORY_BINARY_BOOL, 3},
    {">",  OP_CATEGORY_BINARY_BOOL, 3},
    {">=", OP_CATEGORY_BINARY_BOOL, 3},
    {"<",  OP_CATEGORY_BINARY_BOOL, 3},
    {"<=", OP_CATEGORY_BINARY_BOOL, 3},
    {"+",  OP_CATEGORY_UNARY_MATH,  4},
    {"-",  OP_CATEGORY_UNARY_MATH,  4},
    {"*",  OP_CATEGORY_BINARY_MATH, 5},
    {"/",  OP_CATEGORY_BINARY_MATH, 5},
    {"%",  OP_CATEGORY_BINARY_MATH, 5},
    {"+",  OP_CATEGORY_UNARY_MATH,  6},
    {"-",  OP_CATEGORY_UNARY_MATH,  6},
    {"^",  OP_CATEGORY_BINARY_MATH, 7},
    {"(",  OP_CATEGORY_PAREN,       8},
    {")",  OP_CATEGORY_PAREN,       8},
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
