#include <stdbool.h>

#include <utf8proc.h>

#include "rune.h"

bool rune_is_alpha(rune r) {
    utf8proc_category_t category = utf8proc_category(r);

    switch (category) {
        case UTF8PROC_CATEGORY_LU:
        case UTF8PROC_CATEGORY_LL:
        case UTF8PROC_CATEGORY_LT:
        case UTF8PROC_CATEGORY_LM:
        case UTF8PROC_CATEGORY_LO: {
            return true;
        }
        default: {
            return false;
        }
    }
}

bool rune_is_digit(rune r) {
    return utf8proc_category(r) == UTF8PROC_CATEGORY_ND;
}

bool rune_is_whitespace(rune r) {
    utf8proc_category_t category = utf8proc_category(r);

    switch (category) {
        case UTF8PROC_CATEGORY_ZS:
        case UTF8PROC_CATEGORY_ZL:
        case UTF8PROC_CATEGORY_ZP: {
            return true;
        }
        default: {
            return false;
        }
    }
}

