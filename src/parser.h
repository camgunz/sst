#ifndef PARSER_H__
#define PARSER_H__

enum {
    PARSER_INVALID_SYNTAX = 1,
};

typedef enum {
    EXPRESSION_NODE_LITERAL;
    EXPRESSION_NODE_OPERATOR;
    EXPRESSION_NODE_LOOKUP;
    EXPRESSION_NODE_FUNCTION_CALL;
    EXPRESSION_NODE_INDEX;
    EXPRESSION_NODE_ARRAY;
} ExpressionNodeType;

typedef struct {
    ExpressionNodeType type;
    union {
        Value literal;
        Operator op;
        SSlice lookup;
        SSlice function;
        SSlice index;
    } as;
} ExpressionNode;

typedef struct {
    Lexer lexer;
    DecimalContext *ctx;
    Array expression_queue;
    Array operator_stack;
} Parser;

bool parser_init(Parser *parser, SSlice *data, DecimalContext *ctx,
                                               Status *status);
void parser_clear(Parser *parser);
bool parser_load_next(Parser *parser, Status *status);

#endif

/* vi: set et ts=4 sw=4: */

