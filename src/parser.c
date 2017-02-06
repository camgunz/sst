#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
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
    PARSER_INVALID_LITERAL_VALUE_TYPE                      \
    "Invalid literal value type"                           \
)

static inline
bool parser_push_literal(Parser *parser, ValueType type, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->expression_queue, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_LITERAL;

    switch (type) {
        case VALUE_NUMBER:
            return value_init_number_from_sslice(
                &node->as.literal,
                &parser->lexer.code_token.as.number
            );
        case VALUE_STRING:
            return value_init_string_from_sslice(
                &node->as.literal,
                &parser->lexer.code_token.as.string
            );
        default:
            return invalid_literal_value_type(status);
    }
}

static inline
bool parser_push_lookup(Parser *parser, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->operator_stack, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_LOOKUP;
    sslice_copy(&node->as.lookup, &parser->lexer.code_token.as.lookup);
}

static inline
bool parser_push_function_call(Parser *parser, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->operator_stack, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_FUNCTION_CALL;
    sslice_copy(&node->as.function, &parser->lexer.code_token.as.function);
}

static inline
bool parser_push_index(Parser *parser, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->operator_stack, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_INDEX;
    sslice_copy(&node->as.index, &parser->lexer.code_token.as.index);
}

static inline
bool parser_push_array(Parser *parser, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->operator_stack, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_ARRAY;
}

static inline
bool parser_push_operator(Parser *parser, Operator op, Status *status) {
    ExpressionNode *node = NULL;

    if (!array_append(&parser->operator_stack, (void **)&node, status)) {
        return false;
    }

    node->type = EXPRESSION_NODE_OPERATOR;
    node->as.op = op;

    return status_ok(status);
}

static
bool parser_parse_keyword(Parser *parser, Status *status) {
    (void)parser;
    (void)status;

    return status_ok(status);
}

static
bool parser_parse_text(Parser *parser, Status *status) {
    (void)parser;
    (void)status;

    return status_ok(status);
}

static
bool parser_parse_expression(Parser *parser, Status *status) {
    ExpressionNode *node = NULL;

    while (true) {
        switch (parser->lexer.code_token.type) {
            case CODE_TOKEN_KEYWORD:
            case CODE_TOKEN_TEXT:
                parser->already_loaded_next = true;
                break;
            case CODE_TOKEN_NUMBER:
                if (!parser_push_literal(parser, VALUE_NUMBER, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_STRING:
                if (!parser_push_literal(parser, VALUE_STRING, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_LOOKUP:
                if (!parser_push_lookup(parser, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_FUNCTION_START:
                if (!parser_push_function_call(parser, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_INDEX_END:
                do { /* I mean to go one past the last LITERAL here */
                    if (!array_append(&parser->expression_queue,
                                      (void **)&node,
                                      status)) {
                        return false;
                    }

                    if (!array_pop_right(&parser->operator_stack,
                                         (void **)&node;
                                         status)) {
                        return false;
                    }
                } while (node->type == EXPRESSION_NODE_LITERAL);
                break;
            case CODE_TOKEN_OPERATOR:
                switch (parser->lexer.code_token.as.op) {
                    case OP_OPAREN:
                        if (!parser_push_operator(parser, OP_OPAREN, status)) {
                            return false;
                        }
                        break;
                    case OP_CPAREN:
                        do { /* I mean to go one past the last LITERAL here */
                            if (!array_append(&parser->expression_queue,
                                              (void **)&node,
                                              status)) {
                                return false;
                            }

                            if (!array_pop_right(&parser->operator_stack,
                                                 (void **)&node;
                                                 status)) {
                                return false;
                            }
                        } while (node->type == EXPRESSION_NODE_LITERAL);
                        break;
                    default:
                        while (parser->operator_stack.len > 0) {
                            if (!array_index(&parser->operator_stack,
                                             parser->operator_stack.len - 1,
                                             (void **)&node,
                                             status)) {
                                return false;
                            }

                            if (node->type != EXPRESSION_NODE_OPERATOR) {
                                break;
                            }

                            OperatorInfo *op1 = &OperatorInfo[op];
                            OperatorInfo *op2 = &OperatorInfo[node->as.op];

                            if (((op1->assoc == OP_ASSOC_LEFT) &&
                                 (op1->prec <= op2->prec)) ||
                                ((op1->assoc == OP_ASSOC_RIGHT) &&
                                 (op1->prec < op2->prec))) {
                                ExpressionNode *node2 = NULL;

                                if (!array_append(&parser->operator_stack,
                                                  (void **)&node2,
                                                  status)) {
                                    return false;
                                }

                                if (!array_pop_right(&parser->expression_queue,
                                                     (void **)&node2;
                                                     status)) {
                                    return false;
                                }
                            }
                        }
                        break;
                }
                break;
            case CODE_TOKEN_FUNCTION_ARGUMENT_END:
                if (!array_index(&parser->operator_stack,
                                 parser->operator_stack.len - 1,
                                 (void **)&node,
                                 status)) {
                    return false;
                }
                while (node->type != EXPRESSION_NODE_FUNCTION_CALL) {
                    ExpressionNode *node2 = NULL;

                    if (!array_append(&parser->expression_queue,
                                      (void **)&node2,
                                      status)) {
                        return false;
                    }

                    if (!array_pop_right(&parser->operator_stack,
                                         (void **)&node2;
                                         status)) {
                        return false;
                    }
                }
                break;
            case CODE_TOKEN_INDEX_START:
                if (!parser_push_index(parser, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_ARRAY_START:
                if (!parser_push_array(parser, status)) {
                    return false;
                }
                break;
            case CODE_TOKEN_OPERATOR:
                if (!parser_push_operator(parser, status)) {
                    return false;
                }
                break;
            default:
                break;
        }
    }

    return status_ok(status);
}

bool parser_init(Parser *parser, SSlice *data, DecimalContext *ctx,
                                               Status *status) {
    if (!lexer_init(&parser->lexer, data, status)) {
        return false;
    }

    parser->ctx = ctx;

    return status_ok(status);
}

void parser_clear(Parser *parser) {
    lexer_clear(&parser->lexer);

    parser->ctx = NULL;
}

bool parser_load_next(Parser *parser, Status *status) {
    if (!lexer_load_next(&parser->lexer, status)) {
        return false;
    }

    switch (parser->lexer.code_token.type) {
        case CODE_TOKEN_KEYWORD:
            return parser_parse_keyword(parser, status);
        case CODE_TOKEN_TEXT:
             return parser_parse_text(parser, status);
        case CODE_TOKEN_NUMBER:
        case CODE_TOKEN_STRING:
        case CODE_TOKEN_LOOKUP:
        case CODE_TOKEN_FUNCTION_START:
        case CODE_TOKEN_INDEX_START:
        case CODE_TOKEN_ARRAY_START:
        case CODE_TOKEN_OPERATOR:
            return parser_parse_expression(parser, status);
        default:
            break;
    }

    return invalid_syntax(status);
}

/* vi: set et ts=4 sw=4: */
