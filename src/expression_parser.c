#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "expression_parser.h"
#include "parser.h"

#define EXPRESSION_PARSER_INIT_ALLOC 16

#define unexpected_comma(status) status_failure( \
    status,                                      \
    "expression parser",                         \
    EXPRESSION_PARSER_UNEXPECTED_COMMA,          \
    "Unexpected comma"                           \
)

#define unmatched_parenthesis(status) status_failure( \
    status,                                           \
    "expression parser",                              \
    EXPRESSION_PARSER_UNMATCHED_PARENTHESIS,          \
    "Unmatched parenthesis"                           \
)

#define unmatched_function_end(status) status_failure( \
    status,                                            \
    "expression parser",                               \
    EXPRESSION_PARSER_UNMATCHED_FUNCTION_END,          \
    "Unmatched function end"                           \
)

#define unmatched_array_end(status) status_failure( \
    status,                                         \
    "expression parser",                            \
    EXPRESSION_PARSER_UNMATCHED_ARRAY_END,          \
    "Unmatched array end"                           \
)

#define unmatched_index_end(status) status_failure( \
    status,                                         \
    "expression parser",                            \
    EXPRESSION_PARSER_UNMATCHED_INDEX_END,          \
    "Unmatched index end"                           \
)

#define expected_operator(status) status_failure( \
    status,                                       \
    "expression parser",                          \
    EXPRESSION_PARSER_EXPECTED_OPERATOR,          \
    "Expected operator"                           \
)

#define extraneous_parentheses(status) status_failure( \
    status,                                            \
    "expression parser",                               \
    EXPRESSION_PARSER_EXTRANEOUS_PARENTHESES,          \
    "Extraneous parentheses"                           \
)

bool expression_parser_convert_to_rpn(ExpressionParser *expression_parser,
                                      Status *status) {
    /*
     * This just runs shunting yard.  We need function arity information to go
     * any further (fold an expression, generate instructions like LOAD, EVAL,
     * whatever).  Folding and type checking has to happen during compilation,
     * where functions must be available.
     */

    Array *code_tokens = &expression_parser->code_tokens;
    PArray *operators = &expression_parser->operators;
    PArray *output = &expression_parser->output;

    parray_clear(operators);
    if (!parray_ensure_capacity(operators, code_tokens->len, status)) {
        return false;
    }

    parray_clear(output);
    if (!parray_ensure_capacity(output, code_tokens->len, status)) {
        return false;
    }

    for (size_t i = 0; i < code_tokens->len; i++) {
        CodeToken *code_token = array_index_fast(code_tokens, i);

        if ((code_token->type == CODE_TOKEN_NUMBER) ||
            (code_token->type == CODE_TOKEN_STRING)) {
            if (!parray_append(output, code_token, status)) {
                return false;
            }
        }
        else if ((code_token->type == CODE_TOKEN_LOOKUP) ||
                 (code_token->type == CODE_TOKEN_FUNCTION_START) ||
                 (code_token->type == CODE_TOKEN_INDEX_START) ||
                 (code_token->type == CODE_TOKEN_ARRAY_START)) {
            if (!parray_append(operators, code_token, status)) {
                return false;
            }
        }
        else if (code_token->type == CODE_TOKEN_FUNCTION_ARGUMENT_END) {
            bool found_function_start = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_FUNCTION_START) {
                    found_function_start = true;
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_function_start) {
                return unexpected_comma(status);
            }
        }
        else if (code_token->type == CODE_TOKEN_ARRAY_ELEMENT_END) {
            bool found_array_start = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_ARRAY_START) {
                    found_array_start = true;
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_array_start) {
                return unexpected_comma(status);
            }
        }
        else if ((code_token->type == CODE_TOKEN_OPERATOR) &&
                 (code_token->as.op == OP_OPAREN)) {
            if (!parray_append(operators, code_token, status)) {
                return false;
            }
        }
        else if ((code_token->type == CODE_TOKEN_OPERATOR) &&
                 (code_token->as.op == OP_CPAREN)) {
            bool found_open_parenthesis = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if ((op->type == CODE_TOKEN_OPERATOR) &&
                    (op->as.op == OP_OPAREN)) {
                    found_open_parenthesis = true;
                    parray_truncate_fast(operators, operators->len - 1);
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_open_parenthesis) {
                return unmatched_parenthesis(status);
            }
        }
        else if (code_token->type == CODE_TOKEN_FUNCTION_END) {
            bool found_function_start = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_FUNCTION_START) {
                    found_function_start = true;
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_function_start) {
                return unmatched_function_end(status);
            }
        }
        else if (code_token->type == CODE_TOKEN_ARRAY_END) {
            bool found_array_start = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_ARRAY_START) {
                    found_array_start = true;
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_array_start) {
                return unmatched_array_end(status);
            }
        }
        else if (code_token->type == CODE_TOKEN_INDEX_END) {
            bool found_index_start = false;

            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_INDEX_START) {
                    found_index_start = true;
                    break;
                }

                if (!parray_append(output, op, status)) {
                    return false;
                }

                parray_truncate_fast(operators, operators->len - 1);
            }

            if (!found_index_start) {
                return unmatched_index_end(status);
            }
        }
        else {
            while (operators->len > 0) {
                CodeToken *op = parray_index_fast(
                    operators,
                    operators->len - 1
                );

                if (op->type == CODE_TOKEN_LOOKUP) {
                    if (!parray_append(output, op, status)) {
                        return false;
                    }

                    parray_truncate_fast(operators, operators->len - 1);

                    break;
                }

                if ((op->type == CODE_TOKEN_FUNCTION_START) ||
                    (op->type == CODE_TOKEN_INDEX_START) ||
                    (op->type == CODE_TOKEN_ARRAY_START)) {

                    break;
                }

                if (code_token->type != CODE_TOKEN_OPERATOR) {
                    return expected_operator(status);
                }

                if (op->type != CODE_TOKEN_OPERATOR) {
                    return expected_operator(status);
                }

                OperatorInformation *op1nfo = &OperatorInfo[code_token->as.op];
                OperatorInformation *op2nfo = &OperatorInfo[op->as.op];
                OperatorAssociativity op1_assoc = op1nfo->assoc;
                unsigned char op1_prec = op1nfo->prec;
                unsigned char op2_prec = op2nfo->prec;

                if (((op1_assoc == OP_ASSOC_LEFT) &&
                     (op1_prec <= op2_prec)) ||
                    ((op1_assoc == OP_ASSOC_RIGHT) &&
                     (op1_prec < op2_prec))) {

                    if (!parray_append(output, op, status)) {
                        return false;
                    }

                    parray_truncate_fast(operators, operators->len - 1);
                }
                else {
                    break;
                }

                if (operators->len >= 1) {
                    op = parray_index_fast(operators, operators->len - 1);
                    if ((op->type == CODE_TOKEN_FUNCTION_START) ||
                        (op->type == CODE_TOKEN_INDEX_START) ||
                        (op->type == CODE_TOKEN_ARRAY_START)) {
                        break;
                    }
                }
            }

            if (!parray_append(operators, code_token, status)) {
                return false;
            }
        }
    }

    while (operators->len > 0) {
        CodeToken *op = parray_index_fast(operators, operators->len - 1);

        if ((op->type == CODE_TOKEN_OPERATOR) &&
            ((op->as.op == OP_OPAREN) || (op->as.op == OP_CPAREN))) {
            return extraneous_parentheses(status);
        }

        if (!parray_append(output, op, status)) {
            return false;
        }

        parray_truncate_fast(operators, operators->len - 1);
    }

    return status_ok(status);
}

bool expression_parser_init(ExpressionParser *expression_parser,
                            Status *status) {
    if (!array_init_alloc(&expression_parser->code_tokens,
                          sizeof(CodeToken),
                          EXPRESSION_PARSER_INIT_ALLOC,
                          status)) {
        return false;
    }

    if (!parray_init_alloc(&expression_parser->operators,
                           EXPRESSION_PARSER_INIT_ALLOC,
                           status)) {
        return false;
    }

    if (!parray_init_alloc(&expression_parser->output,
                           EXPRESSION_PARSER_INIT_ALLOC,
                           status)) {
        return false;
    }

    return status_ok(status);
}

void expression_parser_clear(ExpressionParser *expression_parser) {
    array_clear(&expression_parser->code_tokens);
    parray_clear(&expression_parser->operators);
    parray_clear(&expression_parser->output);
}

void expression_parser_free(ExpressionParser *expression_parser) {
    array_free(&expression_parser->code_tokens);
    parray_free(&expression_parser->operators);
    parray_free(&expression_parser->output);
}

/* vi: set et ts=4 sw=4: */
