#ifndef EXPRESSION_H__
#define EXPRESSION_H__

typedef enum {
    OPERAND_LITERAL,
    OPERAND_IDENTIFIER,
    OPERAND_INDEX,
    OPERAND_LOOKUP,
    OPERAND_FUNCTION_CALL,
} OperandType;

typedef struct {
    SSlice identifier;
    size_t index;
} ArrayIndex;

typedef struct {
    OperandType type;
    union {
        Value literal;
        SSlice identifier;
        ArrayIndex array_index;
        SSlice object_lookup;
        FunctionCall *function_call;
    } as;
} OperandValue;

typedef enum {
    EXPRESSION_LITERAL = 1,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
} ExpressionType;

typedef struct {
    Operand operand;
} LiteralExpression;

typedef struct {
    Operator op;
    Operand operand;
} UnaryExpression;

typedef struct {
    Operand operand1;
    Operator op;
    Operand operand2;
} BinaryExpression;

typedef struct {
    ExpressionType type;
    union {
        LiteralExpression *literal_expression;
        UnaryExpression *unary_expression;
        BinaryExpression *binary_expression;
    } as;
} Expression;

#endif

/* vi: set et ts=4 sw=4: */
