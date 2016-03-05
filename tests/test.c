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
#define NUMBER3 "96331332867568925804626874710826851062529687.085"
#define NUMBER4      "023489234902342323419041892349034189341.796"
#define NUMBER5       "23489234902342323419041892349034189341.796"
#define NUMBER6       "23489234902342323419041892349034189341.79"

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
#define STRING  "'This is a string'"

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

    if (strcmp(sn, "82349023489234902342323419041892349034189341.796") != 0) {
        die("Failed converting number to string\n");
    }

    free(sn);
    mpd_del(n);
}

void test_sslice(void) {
    SSlice s;
    SSlice s2;
    char *cs = strdup(NUMBER1);
    rune r;

    sslice_assign(&s, cs);

    if (!sslice_equals(&s, cs)) {
        die("sslice_equals failed\n");
    }

    if (!sslice_starts_with(&s, "8234")) {
        die("sslice_starts_with failed\n");
    }

    if (s.len != 48) {
        die("sslice length != 48 (%zu)\n", s.len);
    }

    sslice_get_first_rune(&s, &r);

    if (r != '8') {
        die("First rune was not '8'\n");
    }

    r = 0;

    sslice_pop_rune(&s, &r);

    if (r != '8') {
        die("Popped rune was not '8'\n");
    }

    if (s.len != 47) {
        die("sslice length != 47) (%zu)\n", s.len);
    }

    if (!sslice_first_rune_equals(&s, '2')) {
        die("Next rune was not '2'\n");
    }

    sslice_pop_rune_if_equals(&s, '2');

    if (sslice_first_rune_equals(&s, '2')) {
        die("Failed to pop rune '2'\n");
    }

    if (!sslice_first_rune_equals(&s, '3')) {
        die("Third rune was not '3'\n");
    }

    if (s.len != 46) {
        die("sslice length != 46\n");
    }

    sslice_pop_rune_if_digit(&s, &r);

    if (r != '3') {
        die("Popped rune was not 3\n");
    }

    if (s.len != 45) {
        die("sslice length != 45\n");
    }

    sslice_pop_rune_if_alnum(&s, &r);

    if (r != '4') {
        die("Popped rune was not 4\n");
    }

    if (s.len != 44) {
        die("sslice length != 44\n");
    }

    sslice_seek_to(&s, '0');

    if (!sslice_first_rune_equals(&s, '0')) {
        die("Failed to seek to '0'\n");
    }

    if (!sslice_equals(&s, NUMBER4)) {
        die("2nd sslice_equals failed\n");
    }

    sslice_shallow_copy(&s2, &s);

    sslice_advance_rune(&s2);

    if (!sslice_equals(&s2, NUMBER5)) {
        die("Advancing a rune failed\n");
    }

    sslice_truncate_at_subslice(&s, &s2);

    if (!sslice_equals(&s, "0")) {
        die("sslice_truncate_at_subslice failed\n");
    }

    sslice_truncate_rune(&s2);

    if (!sslice_equals(&s2, NUMBER6)) {
        die("Truncating a rune failed\n");
    }

    sslice_assign(&s, NUMBERS);

    sslice_advance_rune(&s);

    sslice_truncate_at_whitespace(&s);

    if (!sslice_equals(&s, "14")) {
        die("Truncating at whitespace failed\n");
    }

    sslice_assign(&s, STRING);

    sslice_advance_rune(&s);

    sslice_truncate_at(&s, '\'');

    if (!sslice_equals(&s, "This is a string")) {
        die("Truncating at rune failed\n");
    }
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

    if (strcmp(result, NUMBER3) != 0) {
        die("Addition failed\n");
    }
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
        char *token_as_string;
        
        lstatus = lexer_load_next(&lexer);

        if (lstatus != LEXER_OK) {
            break;
        }

        token = lexer_get_current_token(&lexer);

        if (token->type == TOKEN_UNKNOWN) {
            die("Got unknown token\n");
        }

        token_as_string = token_to_string(token);

        printf("Token: %s [%s]\n", TokenTypes[token->type], token_as_string);

        free(token_as_string);
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
        char *block_as_string;
        pstatus = parser_load_next(&parser);

        if (pstatus != PARSER_OK) {
            break;
        }

        block_as_string = block_to_string(&parser.block);

        printf("Block: %s [%s]\n",
            BlockTypes[parser.block.type], block_as_string
        );

        free(block_as_string);
    }

    if (pstatus != PARSER_OK) {
        die("Bad parser status: %d\n", pstatus);
    }
}

int main(void) {
    printf("Testing MPD... ");
    test_mpd();
    puts("passed");

    printf("Testing addition... ");
    test_add();
    puts("passed");

    printf("Testing SSlice... ");
    test_sslice();
    puts("passed");

    puts("Testing lexer...");
    test_lexer();
    puts("passed");

    puts("Testing parser...");
    test_parser();
    puts("passed");

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */

