#include <stdio.h>
#include <setjmp.h>

#include <cbase.h>

#include <cmocka.h>

#include "lang.h"
#include "tokenizer.h"
#include "lexer.h"
#include "expression_parser.h"
#include "parser.h"

#include "data.h"

void test_parser(void **state) {
    String s;
    SSlice ss;
    Status status;
    Parser parser;

    (void)state;

    status_init(&status);

    assert_true(string_init(&s, TEMPLATE, &status));
    assert_true(string_slice(&s, 0, s.len, &ss, &status));

    assert_true(parser_init(&parser, &ss, &status));

    while (parser_load_next(&parser, &status)) {
        char *cs = NULL;
        char *es = NULL;

        switch (parser.node.type) {
            case AST_NODE_TEXT:
                cs = sslice_to_cstr(&parser.node.as.text);
                printf("<Text: [%s]>\n", cs);
                free(cs);
                break;
            case AST_NODE_INCLUDE:
                cs = sslice_to_cstr(&parser.node.as.include);
                printf("<Include: [%s]>\n", cs);
                free(cs);
                break;
            case AST_NODE_EXPRESSION:
                assert_true(expression_to_cstr(
                    &parser.expression_parser.output,
                    &es,
                    &status
                ));
                printf("<Expression: [%s]>\n", es);
                free(es);
                break;
            case AST_NODE_CONDITIONAL:
                assert_true(expression_to_cstr(
                    &parser.expression_parser.output,
                    &es,
                    &status
                ));
                printf("<Conditional (if): [%s]>\n", es);
                free(es);
                break;
            case AST_NODE_ELSE:
                puts("<Conditional (else)>");
                break;
            case AST_NODE_CONDITIONAL_END:
                puts("<Conditional (end)>");
                break;
            case AST_NODE_ITERATION:
                cs = sslice_to_cstr(&parser.node.as.iteration_identifier);
                assert_true(expression_to_cstr(
                    &parser.expression_parser.output,
                    &es,
                    &status
                ));
                printf("<Iteration: [%s in %s]\n", cs, es);
                free(cs);
                free(es);
                break;
            case AST_NODE_BREAK:
                puts("<Break>");
                break;
            case AST_NODE_CONTINUE:
                puts("<Continue>");
                break;
            case AST_NODE_ITERATION_END:
                puts("<Iteration (end)>");
                break;
            default:
                printf("Unknown node type %d\n", parser.node.type);
                break;
        }
    }

    if (!status_is_ok(&status)) {
        printf("Error in [%s] (%zu: %zu) [file %s, line %d]: %s\n",
            status.domain,
            parser.lexer.tokenizer.line,
            parser.lexer.tokenizer.column,
            status.file,
            status.line,
            status.message
        );
    }

    assert_true(status_is_ok(&status));

    parser_free(&parser);
}

/* vi: set et ts=4 sw=4: */
