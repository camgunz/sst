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

#define NO_ALLOC 1

void print_text_node(ASTNode *node) {
#if NO_ALLOC
    (void)node;
    puts("<Text>");
#else
    char *cs = sslice_to_cstr(&node->as.text);

    printf("<Text: [%s]>\n", cs);
    free(cs);
#endif
}

void print_include_node(ASTNode *node) {
#if NO_ALLOC
    (void)node;
    puts("<Include>");
#else
    char *cs = sslice_to_cstr(&node->as.include);

    printf("<Include: [%s]>\n", cs);
    free(cs);
#endif
}

bool print_expression(ExpressionParser *expression_parser, Status *status) {
#if NO_ALLOC
    (void)expression_parser;
    (void)status;
    puts("<Expression>");
#else
    char *es = NULL;

    if (!expression_to_cstr(&expression_parser->output, &es, status)) {
        return false;
    }

    printf("<Expression: [%s]>\n", es);
    free(es);
#endif

    return status_ok(status);
}

bool print_conditional_node(ExpressionParser *expression_parser,
                            Status *status) {
#if NO_ALLOC
    (void)expression_parser;
    (void)status;
    puts("<Conditional>");
#else
    char *es = NULL;

    if (!expression_to_cstr(&expression_parser->output, &es, status)) {
        return false;
    }

    printf("<Conditional (if): [%s]>\n", es);
    free(es);
#endif

    return status_ok(status);
}

bool print_iteration_node(ASTNode *node, ExpressionParser *expression_parser,
                                         Status *status) {
#if NO_ALLOC
    (void)node;
    (void)expression_parser;
    (void)status;
    puts("<Iteration>");
#else
    char *es = NULL;
    char *cs = NULL;

    cs = sslice_to_cstr(&node->as.iteration_identifier);

    if (!expression_to_cstr(&expression_parser->output, &es, status)) {
        return false;
    }

    printf("<Iteration: [%s in %s]\n", cs, es);
    free(cs);
    free(es);
#endif

    return status_ok(status);
}

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
        switch (parser.node.type) {
            case AST_NODE_TEXT:
                print_text_node(&parser.node);
                break;
            case AST_NODE_INCLUDE:
                print_include_node(&parser.node);
                break;
            case AST_NODE_EXPRESSION:
                if (!print_expression(&parser.expression_parser, &status)) {
                    printf("Error: %s\n", status.message);
                }
                break;
            case AST_NODE_CONDITIONAL:
                if (!print_conditional_node(&parser.expression_parser,
                                            &status)) {
                    printf("Error: %s\n", status.message);
                }
                break;
            case AST_NODE_ELSE:
                puts("<Conditional (else)>");
                break;
            case AST_NODE_CONDITIONAL_END:
                puts("<Conditional (end)>");
                break;
            case AST_NODE_ITERATION:
                if (!print_iteration_node(&parser.node,
                                          &parser.expression_parser,
                                          &status)) {
                    printf("Error: %s\n", status.message);
                }
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

    if (!status_match(&status, "parser", PARSER_EOF)) {
        printf("Error in [%s] (%zu: %zu) [file %s, line %d]: %s\n",
            status.domain,
            parser.lexer.tokenizer.line,
            parser.lexer.tokenizer.column,
            status.file,
            status.line,
            status.message
        );
    }

    assert_true(status_match(&status, "parser", PARSER_EOF));

    parser_free(&parser);
    string_free(&s);

}

/* vi: set et ts=4 sw=4: */
