#ifndef BLOCK_H__
#define BLOCK_H__

typedef enum {
    BLOCK_OK = SST_OK,
} BlockStatus;

typedef enum {
    BLOCK_TEXT,
    BLOCK_INCLUDE,
    BLOCK_EXPRESSION,
    BLOCK_CONDITIONAL,
    BLOCK_ITERATION,
    BLOCK_MAX
} BlockType;

typedef SSlice TextBlock;

typedef struct {
    SSlice path;
} IncludeBlock;

typedef struct {
    Expression expression;
} ExpressionBlock;

typedef enum {
    BOOLEAN_EXPRESSION_UNARY,
    BOOLEAN_EXPRESSION_BINARY,
    BOOLEAN_EXPRESSION_MAX
} BooleanExpressionType;

typedef struct {
    BooleanExpressionType type;
    union {
        UnaryBooleanExpression unary_boolean_expression;
        BooleanExpression      boolean_expression;
    } as;
} ConditionalBlock;

typedef struct {
    SSlice     identifier;
    Expression sequence;
} IterationBlock;

typedef struct {
    BlockType type;
    union {
        TextBlock        text;
        IncludeBlock     include;
        ExpressionBlock  expression;
        ConditionalBlock conditional;
        IterationBlock   iteration;
    } as;
} Block;

extern const char *BlockTypes[BLOCK_MAX];

char* block_to_string(Block *block);

#endif

/* vi: set et ts=4 sw=4: */

