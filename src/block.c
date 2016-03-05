
#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "token.h"
#include "utils.h"
#include "block.h"

const char *BlockTypes[BLOCK_MAX] = {
    "Text",
    "Include",
    "Expression",
    "Conditional",
    "Iteration"
};

char* block_to_string(Block *block) {
    switch (block->type) {
        case BLOCK_TEXT:
            return sslice_to_c_string(&block->as.text);
        case BLOCK_INCLUDE:
            return sslice_to_c_string(&block->as.include.path);
            break;
        case BLOCK_EXPRESSION:
            return strdup("Expression - WIP");
            break;
        case BLOCK_CONDITIONAL:
            return strdup("Conditional - WIP");
            break;
        case BLOCK_ITERATION:
            return strdup("Iteration - WIP");
            break;
        default:
            return strdup("Unknown");
    }
}

/* vi: set et ts=4 sw=4: */

