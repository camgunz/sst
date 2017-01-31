#include <cbase.h>

#include "config.h"
#include "token.h"
#include "block.h"
#include "lexer.h"
#include "parser.h"

typedef struct {
    Parser *parser;
    Token token;
} Expression;

/*
 * An expression has an array of tokens that it can move through.
 */

static void expression_get_sub_expression(Expression *expression,
                                          unsigned char rbp,
                                          Status *status) {
    Token t;
    
    memcpy(&t, &expression->token, sizeof(Token));

    if (!parser_load_next_token(expression->parser, status)) {
        return false;
    }

    memcpy(
        &expression->token,
        &expression->parser->lexer.token,
        sizeof(Token)
    );

    nud(expression, &t); /* Loads new token in lexer.token */

    while (rbp < lbp(&expression->token)) {
        memcpy(&t, &expression->token, sizeof(Token));

        if (!parser_load_next_token(expression->parser, status)) {
            return false;
        }

        memcpy(
            &expression->token,
            &expression->parser->lexer.token,
            sizeof(Token)
        );
    }
}

/* vi: set et ts=4 sw=4: */
