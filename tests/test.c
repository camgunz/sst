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
#include "token.h"
#include "block.h"
#include "lexer.h"
#include "parser.h"
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
"{{ if message == 'Or you can put them in strings {{ }} }} {{' }}\n"       \
"You guessed the magic message!\n"                                         \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ include '/srv/http/templates/footer.txt' }}\n"                         \
"Last little bit down here\n"

#define NUMBERS " 14 983 2876.55 "

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
    printf("SSlice length: %zu\n", s.len);

    sslice_get_first_rune(&s, &r);
    printf("First char: %c\n", r);

    r = 0;

    sslice_pop_rune(&s, &r);
    printf("Popped first char: %c\n", r);
    printf("SSlice length: %zu\n", s.len);

    printf("Second char is '2': %u\n", sslice_first_rune_equals(&s, '2'));

    sslice_pop_rune_if_equals(&s, '2');
    printf("Second char: %u\n", sslice_first_rune_equals(&s, '2'));
    printf("SSlice length: %zu\n", s.len);

    sslice_pop_rune_if_digit(&s, &r);
    printf("Third char: %c\n", r);
    printf("SSlice length: %zu\n", s.len);

    sslice_pop_rune_if_alnum(&s, &r);
    printf("Fourth char: %c\n", r);
    printf("SSlice length: %zu\n", s.len);

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

void test_lexer(void) {
    Lexer lexer;
    SSlice data;
    SSliceStatus sstatus;
    LexerStatus lstatus;

    sstatus = sslice_assign_validate(&data, REAL_TEMPLATE);

    if (sstatus != SSLICE_OK) {
        die("Error loading template into SSlice: %d", sstatus);
    }

    lexer_init(&lexer);
    lexer_set_data(&lexer, &data);

    while (true) {
        Token *token;
        
        lstatus = lexer_load_next(&lexer);

        if (lstatus != LEXER_OK) {
            break;
        }

        token = lexer_get_current_token(&lexer);

        if (token->type == TOKEN_UNKNOWN) {
            die("Got unknown token\n");
        }

        printf("Token: %s [%s]\n",
            TokenTypes[token->type], token_to_string(token)
        );
    }

    if (lstatus == LEXER_END) {
        puts("End of lexer");
    }
    else {
        die("Bad lexer status: %d\n", lstatus);
    }
}

void test_parser(void) {
    Parser parser;
    SSlice data;
    SSliceStatus sstatus;
    ParserStatus pstatus;

    sstatus = sslice_assign_validate(&data, REAL_TEMPLATE);

    if (sstatus != SSLICE_OK) {
        die("Error loading template into SSlice: %d", sstatus);
    }

    parser_init(&parser, &data);

    while (true) {
        pstatus = parser_load_next(&parser);

        if (pstatus != PARSER_OK) {
            break;
        }

        printf("Block: %s\n", BlockTypes[parser.block.type]);
    }

    if (pstatus != PARSER_OK) {
        die("Bad parser status: %d\n", pstatus);
    }
}

int main(void) {
    test_mpd();
    test_sslice();
    test_add();
    test_lexer();
    test_parser();

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */

