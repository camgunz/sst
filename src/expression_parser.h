#ifndef EXPRESSION_PARSER_H__
#define EXPRESSION_PARSER_H__

enum {
    EXPRESSION_PARSER_UNEXPECTED_COMMA = 1,
    EXPRESSION_PARSER_UNMATCHED_PARENTHESIS,
    EXPRESSION_PARSER_UNMATCHED_FUNCTION_END,
    EXPRESSION_PARSER_UNMATCHED_ARRAY_END,
    EXPRESSION_PARSER_UNMATCHED_INDEX_END,
    EXPRESSION_PARSER_EXPECTED_OPERATOR,
    EXPRESSION_PARSER_EXTRANEOUS_PARENTHESES,
};

typedef struct {
    Array code_tokens;
    PArray operators;
    PArray output;
} ExpressionParser;

bool expression_parser_convert_to_rpn(ExpressionParser *expression_parser,
                                      Status *status);
bool expression_parser_init(ExpressionParser *expression_parser,
                            Status *status);
void expression_parser_clear(ExpressionParser *expression_parser);
void expression_parser_free(ExpressionParser *expression_parser);

#endif

/* vi: set et ts=4 sw=4: */

