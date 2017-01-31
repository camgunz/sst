#include <cbase.h>

#include "config.h"
#include "token.h"
#include "utils.h"
#include "expression.h"
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
            return sslice_to_cstr(&block->as.text);
        case BLOCK_INCLUDE:
            return sslice_to_cstr(&block->as.include_path);
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
            break;
    }

    return strdup("Unknown");
}

/* vi: set et ts=4 sw=4: */
