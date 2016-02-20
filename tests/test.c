#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "str.h"
#include "lexer.h"
#include "value.h"
#include "utils.h"

#define NUMBER1 "82349023489234902342323419041892349034189341.796"
#define NUMBER2 "13982309378334023462303455668934502028340345.289"

#define REAL_TEMPLATE \
"{{ include '/srv/http/templates/header.txt' }}\n"                         \
"\n"                                                                       \
"{{ if person.age >= 18 }}\n"                                              \
"...\n"                                                                    \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ for person in people }}\n"                                             \
"    {{ person.name }} owes {{ person.income * tax_rate }} this year!\n"   \
"{{ endfor }}\n"                                                           \
"\n"                                                                       \
"{{ raw }}\n"                                                              \
"    This is how you would use raw }} and {{ markers.\n"                   \
"{{ endraw }}\n"                                                           \
"\n"                                                                       \
"{{ include '/srv/http/templates/footer.txt' }}\n"

#define TEMPLATE " 14 983 2876.55"

void test_mpd(void) {
    mpd_context_t ctx;
    mpd_t *n;
    char *sn = NULL;
    uint32_t res = 0;

    mpd_maxcontext(&ctx);

    n = mpd_qnew();

    if (!n) {
        die("Allocating new number failed\n");
    }

    mpd_qset_string(n, NUMBER1, &ctx, &res);

    sn = mpd_to_sci(n, 0);

    printf("Number: %s\n", sn);

    free(sn);
    mpd_del(n);
}

void test_sslice(void) {
    SSlice s;
    char *cs = strdup(NUMBER1);
    rune r;

    sslice_assign(&s, cs);
    printf("Equals: %u\n", sslice_equals(&s, cs));
    printf("Starts with 8234: %u\n", sslice_starts_with(&s, "8234"));
    printf("SSlice length: %u\n", s.len);

    sslice_get_first_rune(&s, &r);
    printf("First char: %c\n", r);

    r = 0;

    sslice_pop_rune(&s, &r);
    printf("Popped first char: %c\n", r);
    printf("SSlice length: %u\n", s.len);

    printf("Second char is '2': %u\n", sslice_first_rune_equals(&s, '2'));

    sslice_pop_rune_if_equals(&s, '2');
    printf("Second char: %u\n", sslice_first_rune_equals(&s, '2'));
    printf("SSlice length: %u\n", s.len);

    sslice_pop_rune_if_digit(&s, &r);
    printf("Third char: %c\n", r);
    printf("SSlice length: %u\n", s.len);

    sslice_pop_rune_if_alnum(&s, &r);
    printf("Fourth char: %c\n", r);
    printf("SSlice length: %u\n", s.len);

    sslice_seek_to(&s, '0');
    printf("Find 0: %s\n", s.data);
}

void test_add(void) {
    String s1;
    String s2;
    Value v1;
    Value v2;
    Value v3;
    char *v1s;
    char *v2s;
    char *result;
    mpd_context_t mpd_ctx;

    mpd_maxcontext(&mpd_ctx);

    string_init(&s1, NUMBER1);
    string_init(&s2, NUMBER2);

    value_init(&v1, &mpd_ctx);
    value_init(&v2, &mpd_ctx);
    value_init(&v3, &mpd_ctx);

    value_set_number(&v1, &s1);
    value_set_number(&v2, &s2);

    value_add(&v3, &v1, &v2);

    value_as_string(&v1s, &v1);
    value_as_string(&v2s, &v2);

    value_as_string(&result, &v3);

    printf("Result: %s + %s = %s\n", v1s, v2s, result);
}

#if 0
void test_splitter(void) {
    Splitter splitter;

    splitter_init(&splitter, TEMPLATE);

    while (splitter_load_next(&splitter)) {
        printf("Section - is_code: %u\n", splitter.section_is_code);
        if (splitter.section_is_code) {
            char *code = strndup(splitter.section.data, splitter.section.len);

            printf("Code: [%s]\n", code);

            free(code);
        }
    }
}
#endif

static const char *token_types[TOKEN_MAX] = {
    "Unknown",
    "Number",
    "Keyword",
    "Identifier",
    "String",
    "BoolOp",
    "Unary BoolOp",
    "MathOp",
    "Symbol",
    "Whitespace"
};

static char* token_to_string(Token *token) {
    switch (token->type) {
        case TOKEN_NUMBER: {
            return mpd_to_sci(token->as.number, 0);
        }
        case TOKEN_KEYWORD:
            return strdup(KeywordValues[token->as.keyword]);
        case TOKEN_IDENTIFIER:
            return sslice_to_c_string(&token->as.identifier);
        case TOKEN_STRING:
            return sslice_to_c_string(&token->as.string);
        case TOKEN_BOOLOP:
            return strdup(BoolOpValues[token->as.bool_op]);
        case TOKEN_UNARY_BOOLOP:
            return chardup('!');
        case TOKEN_MATHOP:
            return chardup(MathOpValues[token->as.math_op]);
        case TOKEN_SYMBOL:
            return chardup(SymbolValues[token->as.symbol]);
        case TOKEN_WHITESPACE:
            switch (token->as.whitespace) {
                case WHITESPACE_SPACE:
                    return strdup("<space>");
                case WHITESPACE_TAB:
                    return strdup("<tab>");
                case WHITESPACE_CARRIAGE_RETURN:
                    return strdup("<cr>");
                case WHITESPACE_NEWLINE:
                    return strdup("<nl>");
                case WHITESPACE_MAX:
                default:
                    return strdup("<ws_unknown>");
            }
            break;
        case TOKEN_UNKNOWN:
        case TOKEN_MAX:
        default:
            return strdup("Unknown");
    }
}

void test_lexer(void) {
    Lexer lexer;
    SSlice data;
    SSliceStatus status;
    LexerStatus lstatus;

    status = sslice_assign_validate(&data, TEMPLATE);

    if (status != SSLICE_OK) {
        die("Bad status: %d\n", status);
    }

    printf("Data: %s\n", data.data);

    lexer_init(&lexer);
    lexer_set_data(&lexer, &data);

    printf("Lexer data: %s\n", lexer.data.data);

    while (true) {
        lstatus = lexer_load_next_skip_whitespace(&lexer);
        Token *token = lexer_get_current_token(&lexer);

        if (lstatus != LEXER_OK) {
            break;
        }

        if (token->type == TOKEN_UNKNOWN) {
            die("Got unknown token\n");
        }

        printf("Token: %s [%s]\n",
            token_types[token->type], token_to_string(token)
        );
    }

    if (lstatus == LEXER_END) {
        puts("End of lexer");
    }
    else {
        die("Bad lexer status: %d\n", lstatus);
    }
}

int main(void) {
    test_mpd();
    test_sslice();
    test_add();
#if 0
    test_splitter();
#endif
    test_lexer();

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */

