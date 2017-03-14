#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "value.h"
#include "tokenizer.h"
#include "lexer.h"
#include "expression_parser.h"
#include "parser.h"

#define invalid_syntax(status) status_failure( \
    status,                                    \
    "parser",                                  \
    PARSER_INVALID_SYNTAX,                     \
    "Invalid syntax"                           \
)

#define invalid_literal_value_type(status) status_failure( \
    status,                                                \
    "parser",                                              \
    PARSER_INVALID_LITERAL_VALUE_TYPE,                     \
    "Invalid literal value type"                           \
)

#define unexpected_comma(status) status_failure( \
    status,                                      \
    "parser",                                    \
    PARSER_UNEXPECTED_COMMA,                     \
    "Unexpected comma"                           \
)

#define unexpected_keyword_in(status) status_failure( \
    status,                                           \
    "parser",                                         \
    PARSER_UNEXPECTED_KEYWORD_IN,                     \
    "Unexpected \"in\" keyword"                       \
)

#define unmatched_parenthesis(status) status_failure( \
    status,                                           \
    "parser",                                         \
    PARSER_UNMATCHED_PARENTHESIS,                     \
    "Unmatched parenthesis"                           \
)

#define unmatched_function_end(status) status_failure( \
    status,                                            \
    "parser",                                          \
    PARSER_UNMATCHED_FUNCTION_END,                     \
    "Unmatched function end"                           \
)

#define unmatched_array_end(status) status_failure( \
    status,                                         \
    "parser",                                       \
    PARSER_UNMATCHED_ARRAY_END,                     \
    "Unmatched array end"                           \
)

#define unmatched_index_end(status) status_failure( \
    status,                                         \
    "parser",                                       \
    PARSER_UNMATCHED_INDEX_END,                     \
    "Unmatched index end"                           \
)

#define expected_expression(status) status_failure( \
    status,                                         \
    "parser",                                       \
    PARSER_EXPECTED_STRING,                         \
    "Expected expression"                           \
)

#define expected_string(status) status_failure( \
    status,                                     \
    "parser",                                   \
    PARSER_EXPECTED_STRING,                     \
    "Expected string"                           \
)

#define expected_identifier(status) status_failure( \
    status,                                         \
    "parser",                                       \
    PARSER_EXPECTED_IDENTIFIER,                     \
    "Expected identifier"                           \
)

#define expected_keyword_in(status) status_failure( \
    status,                                         \
    "parser",                                       \
    PARSER_EXPECTED_KEYWORD_IN,                     \
    "Expected \"in\" keyword"                       \
)

#define expected_operator(status) status_failure( \
    status,                                       \
    "parser",                                     \
    PARSER_EXPECTED_OPERATOR,                     \
    "Expected operator"                           \
)

#define else_without_if(status) status_failure( \
    status,                                     \
    "parser",                                   \
    PARSER_ELSE_WITHOUT_IF,                     \
    "\"else\" without \"if\""                   \
)

#define endif_without_if(status) status_failure( \
    status,                                      \
    "parser",                                    \
    PARSER_ENDIF_WITHOUT_IF,                     \
    "\"endif\" without \"if\""                   \
)

#define break_without_for(status) status_failure( \
    status,                                       \
    "parser",                                     \
    PARSER_BREAK_WITHOUT_FOR,                     \
    "\"break\" without \"for\""                   \
)

#define continue_without_for(status) status_failure( \
    status,                                          \
    "parser",                                        \
    PARSER_CONTINUE_WITHOUT_FOR,                     \
    "\"continue\" without \"for\""                   \
)

#define endfor_without_for(status) status_failure( \
    status,                                        \
    "parser",                                      \
    PARSER_ENDFOR_WITHOUT_FOR,                     \
    "\"endfor\" without \"for\""                   \
)

#define extraneous_parentheses(status) status_failure( \
    status,                                            \
    "parser",                                          \
    PARSER_EXTRANEOUS_PARENTHESES,                     \
    "Extraneous parentheses"                           \
)

#define unknown_keyword(status) status_failure( \
    status,                                     \
    "parser",                                   \
    PARSER_UNKNOWN_KEYWORD,                     \
    "Unknown keyword"                           \
)

static
bool parser_parse_expression(Parser *parser, Status *status) {
    /*
     * This just runs shunting yard.  We need function arity information to go
     * any further (fold an expression, generate instructions like LOAD, EVAL,
     * whatever).  Folding and type checking has to happen during compilation,
     * where functions must be available.
     */

    Array *code_tokens = &parser->expression_parser.code_tokens;

    array_clear(code_tokens);

    while (true) {
        CodeToken *code_token = NULL;

        switch (parser->lexer.code_token.type) {
            case CODE_TOKEN_KEYWORD:
            case CODE_TOKEN_TEXT:
                goto tokens_gathered;
                break;
            default:
                break;
        }

        if (!array_append(code_tokens, (void **)&code_token, status)) {
            return false;
        }

        code_token->type = parser->lexer.code_token.type;

        switch (parser->lexer.code_token.type) {
            case CODE_TOKEN_TEXT:
                sslice_copy(
                    &code_token->as.text,
                    &parser->lexer.code_token.as.text
                );
                break;
            case CODE_TOKEN_NUMBER:
                sslice_copy(
                    &code_token->as.number,
                    &parser->lexer.code_token.as.number
                );
                break;
            case CODE_TOKEN_STRING:
                sslice_copy(
                    &code_token->as.string,
                    &parser->lexer.code_token.as.string
                );
                break;
            case CODE_TOKEN_KEYWORD:
                code_token->as.keyword = parser->lexer.code_token.as.keyword;
                break;
            case CODE_TOKEN_LOOKUP:
                sslice_copy(
                    &code_token->as.lookup,
                    &parser->lexer.code_token.as.lookup
                );
                break;
            case CODE_TOKEN_FUNCTION_START:
                sslice_copy(
                    &code_token->as.function,
                    &parser->lexer.code_token.as.function
                );
                break;
            case CODE_TOKEN_INDEX_START:
                sslice_copy(
                    &code_token->as.index,
                    &parser->lexer.code_token.as.index
                );
                break;
            case CODE_TOKEN_OPERATOR:
                code_token->as.op = parser->lexer.code_token.as.op;
                break;
            default:
                break;
        }

        if (!lexer_load_next(&parser->lexer, status)) {
            return false;
        }

        parser->already_loaded_next = true;
    }

tokens_gathered:

    return expression_parser_convert_to_rpn(
        &parser->expression_parser,
        status
    );
}

static
bool parser_parse_keyword(Parser *parser, Status *status) {
    SSlice iteration_identifier;

    switch (parser->lexer.code_token.as.keyword) {
        case KEYWORD_INCLUDE:
            if (!lexer_load_next(&parser->lexer, status)) {
                return false;
            }

            if (parser->lexer.code_token.type != CODE_TOKEN_STRING) {
                return expected_string(status);
            }

            parser->node.type = AST_NODE_INCLUDE;

            sslice_copy(
                &parser->node.as.include,
                &parser->lexer.code_token.as.string
            );

            break;
        case KEYWORD_IF:
            parser->conditional_depth++;

            if (!lexer_load_next(&parser->lexer, status)) {
                return false;
            }

            if ((parser->lexer.code_token.type != CODE_TOKEN_NUMBER) &&
                (parser->lexer.code_token.type != CODE_TOKEN_STRING) &&
                (parser->lexer.code_token.type != CODE_TOKEN_LOOKUP) &&
                (parser->lexer.code_token.type != CODE_TOKEN_FUNCTION_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_INDEX_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_ARRAY_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_OPERATOR)) {
                return expected_expression(status);
            }

            if (!parser_parse_expression(parser, status)) {
                return false;
            }

            parser->node.type = AST_NODE_CONDITIONAL;

            break;
        case KEYWORD_ELSE:
            if (parser->conditional_depth == 0) {
                return else_without_if(status);
            }

            parser->node.type = AST_NODE_ELSE;

            break;
        case KEYWORD_ENDIF:
            if (parser->conditional_depth == 0) {
                return endif_without_if(status);
            }

            parser->conditional_depth--;

            parser->node.type = AST_NODE_CONDITIONAL_END;

            break;
        case KEYWORD_FOR:
            parser->iteration_depth++;

            /* expect lookup, KEYWORD_IN, expression */
            if (!lexer_load_next(&parser->lexer, status)) {
                return false;
            }

            if (parser->lexer.code_token.type != CODE_TOKEN_LOOKUP) {
                return expected_identifier(status);
            }

            sslice_copy(
                &iteration_identifier,
                &parser->lexer.code_token.as.lookup
            );

            if (!lexer_load_next(&parser->lexer, status)) {
                return false;
            }

            if ((parser->lexer.code_token.type != CODE_TOKEN_KEYWORD) ||
                (parser->lexer.code_token.as.keyword != KEYWORD_IN)) {
                return expected_keyword_in(status);
            }

            if (!lexer_load_next(&parser->lexer, status)) {
                return false;
            }

            if ((parser->lexer.code_token.type != CODE_TOKEN_NUMBER) &&
                (parser->lexer.code_token.type != CODE_TOKEN_STRING) &&
                (parser->lexer.code_token.type != CODE_TOKEN_LOOKUP) &&
                (parser->lexer.code_token.type != CODE_TOKEN_FUNCTION_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_INDEX_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_ARRAY_START) &&
                (parser->lexer.code_token.type != CODE_TOKEN_OPERATOR)) {
                return expected_expression(status);
            }

            if (!parser_parse_expression(parser, status)) {
                return false;
            }

            parser->node.type = AST_NODE_ITERATION;

            sslice_copy(
                &parser->node.as.iteration_identifier,
                &iteration_identifier
            );

            break;
        case KEYWORD_IN:
            return unexpected_keyword_in(status);
        case KEYWORD_BREAK:
            if (parser->iteration_depth == 0) {
                return break_without_for(status);
            }

            parser->node.type = AST_NODE_BREAK;

            break;
        case KEYWORD_CONTINUE:
            if (parser->iteration_depth == 0) {
                return continue_without_for(status);
            }

            parser->node.type = AST_NODE_CONTINUE;

            break;
        case KEYWORD_ENDFOR:
            if (parser->iteration_depth == 0) {
                return endfor_without_for(status);
            }

            parser->iteration_depth--;

            parser->node.type = AST_NODE_ITERATION_END;

            break;
        default:
            return unknown_keyword(status);
    }

    return status_ok(status);
}

bool parser_init(Parser *parser, SSlice *data, Status *status) {
    if (!lexer_init(&parser->lexer, data, status)) {
        return false;
    }

    if (!expression_parser_init(&parser->expression_parser, status)) {
        return false;
    }

    parser->already_loaded_next = false;
    parser->conditional_depth = 0;
    parser->iteration_depth = 0;

    return status_ok(status);
}

void parser_clear(Parser *parser) {
    lexer_clear(&parser->lexer);

    expression_parser_clear(&parser->expression_parser);

    parser->already_loaded_next = false;
    parser->conditional_depth = 0;
    parser->iteration_depth = 0;
}

void parser_free(Parser *parser) {
    lexer_free(&parser->lexer);

    array_free(&parser->expression_parser.code_tokens);
    parray_free(&parser->expression_parser.operators);
    parray_free(&parser->expression_parser.output);

    parser->already_loaded_next = false;
    parser->conditional_depth = 0;
    parser->iteration_depth = 0;
}

bool parser_load_next(Parser *parser, Status *status) {
    if (!lexer_load_next(&parser->lexer, status)) {
        return false;
    }

    switch (parser->lexer.code_token.type) {
        case CODE_TOKEN_KEYWORD:
            return parser_parse_keyword(parser, status);
        case CODE_TOKEN_TEXT:
            parser->node.type = AST_NODE_TEXT;

            sslice_copy(
                &parser->node.as.text,
                &parser->lexer.code_token.as.text
            );

            return status_ok(status);
        case CODE_TOKEN_NUMBER:
        case CODE_TOKEN_STRING:
        case CODE_TOKEN_LOOKUP:
        case CODE_TOKEN_FUNCTION_START:
        case CODE_TOKEN_INDEX_START:
        case CODE_TOKEN_ARRAY_START:
        case CODE_TOKEN_OPERATOR:
            if (!parser_parse_expression(parser, status)) {
                return false;
            }

            parser->node.type = AST_NODE_EXPRESSION;

            return status_ok(status);
        default:
            break;
    }

    return invalid_syntax(status);
}

bool expression_to_string(PArray *expression, String *str, Status *status) {
    for (size_t i = 0; i < expression->len; i++) {
        CodeToken *code_token = parray_index_fast(expression, i);

        if (i > 0) {
            if (!string_append_cstr(str, ", ", status)) {
                return false;
            }
        }

        if (!code_token_to_string(code_token, str, status)) {
            return false;
        }
    }

    return status_ok(status);
}

bool expression_to_cstr(PArray *expression, char **str, Status *status) {
    String s;

    if (!string_init(&s, "", status)) {
        return false;
    }

    if (!expression_to_string(expression, &s, status)) {
        return false;
    }

    *str = strdup(s.data);

    string_free(&s);

    return status_ok(status);
}

/* vi: set et ts=4 sw=4: */
