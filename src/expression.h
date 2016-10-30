#ifndef EXPRESSION_H__
#define EXPRESSION_H__

/*
 * {{ if (player.shots_made / person.shots_taken) / 100 > .5 }}
 * Holy cow!
 * {{ endif }}
 *
 * <keyword:if>
 * <symbol:oparen>
 * <identifier:player.shots_made>
 * <mathop:divide>
 * <identifier:player.shots_taken>
 * <symbol:cparen>
 * <mathop:divide>
 * <boolop:gt>
 * <number:.5>
 */

/*
 * After literal:
 *   - oparen
 *   - cparen
 *   - mathop
 *   - boolop
 *   - comma
 *   - cbracket (only inside sequence literal)
 */

typedef enum {
    OPERAND_STRING,
    OPERAND_NUMBER,
    OPERAND_IDENTIFIER,
    OPERAND_EXPRESSION,
    OPERAND_NOT_EVALUATED,
    OPERAND_MAX
} OperandType;

typedef struct {
    OperandType type;
    union {
        SSlice  string;
        mpd_t  *number;
        SSlice  identifier;
        SSlice  expression;
    } as;
} Operand;

typedef enum {
    RANGE_FLAG_BLANK,
    RANGE_FLAG_HAS_START,
    RANGE_FLAG_HAS_STEP
} RangeFlag;

typedef struct {
    RangeFlag   flags;
    MathOperand start;
    MathOperand stop;
    MathOperand step;
} Range;

typedef enum {
    EXPRESSION_VALUE,
    EXPRESSION_CONDITIONAL,
    EXPRESSION_RANGE

typedef struct {
    ExpressionType type;
    Operand        operand1;
    Operand        operand2;
    MathOp         op;
} Expression;

/*
 * Sequence expressions boil down to:
 *   - Sequence start
 *   - 0 to N expressions
 *   - Sequence end
 *
 * ...so there's no need to define a struct for them.
 */

#endif

/* vi: set et ts=4 sw=4: */

