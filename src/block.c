
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
    "Iteration",
    "Raw"
};

/* vi: set et ts=4 sw=4: */

