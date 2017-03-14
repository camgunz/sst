#ifndef PARSER_H__
#define PARSER_H__

typedef enum {
    AST_NODE_TEXT,
    AST_NODE_INCLUDE,
    AST_NODE_EXPRESSION,
    AST_NODE_CONDITIONAL,
    AST_NODE_ELSE,
    AST_NODE_CONDITIONAL_END,
    AST_NODE_ITERATION,
    AST_NODE_BREAK,
    AST_NODE_CONTINUE,
    AST_NODE_ITERATION_END,
} ASTNodeType;

typedef struct {
    ASTNodeType type;
    union {
        SSlice text;
        SSlice include;
        SSlice iteration_identifier;
    } as;
} ASTNode;

enum {
    PARSER_INVALID_SYNTAX = 1,
    PARSER_INVALID_LITERAL_VALUE_TYPE,
    PARSER_UNEXPECTED_COMMA,
    PARSER_UNEXPECTED_KEYWORD_IN,
    PARSER_UNMATCHED_PARENTHESIS,
    PARSER_UNMATCHED_FUNCTION_END,
    PARSER_UNMATCHED_ARRAY_END,
    PARSER_UNMATCHED_INDEX_END,
    PARSER_EXPECTED_EXPRESSION,
    PARSER_EXPECTED_STRING,
    PARSER_EXPECTED_IDENTIFIER,
    PARSER_EXPECTED_KEYWORD_IN,
    PARSER_EXPECTED_OPERATOR,
    PARSER_ELSE_WITHOUT_IF,
    PARSER_ENDIF_WITHOUT_IF,
    PARSER_BREAK_WITHOUT_FOR,
    PARSER_CONTINUE_WITHOUT_FOR,
    PARSER_ENDFOR_WITHOUT_FOR,
    PARSER_EXTRANEOUS_PARENTHESES,
    PARSER_UNKNOWN_KEYWORD,
};

typedef struct {
    ASTNode node;
    Lexer lexer;
    ExpressionParser expression_parser;
    bool already_loaded_next;
    size_t conditional_depth;
    size_t iteration_depth;
} Parser;

bool parser_init(Parser *parser, SSlice *data, Status *status);
void parser_clear(Parser *parser);
void parser_free(Parser *parser);
bool parser_load_next(Parser *parser, Status *status);

bool expression_to_string(PArray *expression, String *str, Status *status);
bool expression_to_cstr(PArray *expression, char **str, Status *status);

#endif

/* vi: set et ts=4 sw=4: */

