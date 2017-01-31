#ifndef BLOCK_H__
#define BLOCK_H__

typedef enum {
    BLOCK_TEXT,
    BLOCK_INCLUDE,
    BLOCK_CONDITIONAL,
    BLOCK_ITERATION,
    BLOCK_MAX
} BlockType;

typedef struct {
    SSlice     identifier;
    Expression sequence;
} IterationBlock;

typedef struct {
    BlockType type;
    union {
        SSlice         text;
        SSlice         include;
        Expression     conditional;
        IterationBlock iteration;
    } as;
} Block;

extern const char *BlockTypes[BLOCK_MAX];

char* block_to_string(Block *block);

#endif

/* vi: set et ts=4 sw=4: */

