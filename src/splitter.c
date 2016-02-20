#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "utils.h"
#include "sslice.h"
#include "splitter.h"

void splitter_clear(Splitter *splitter) {
    string_clear(&splitter->data);
    string_clear(&splitter->section);
    splitter->section_is_code = false;
}

void splitter_init(Splitter *splitter, String *data) {
    splitter_clear(splitter);
    string_shallow_copy(&splitter->data, data);
}

bool splitter_load_next(Splitter *splitter) {
    String s;

    if (splitter->section.data) {
        s = splitter->section.data + splitter->section.len;
    }
    else {
        s = splitter->data;
    }

    if (empty_string(s)) {
        return false;
    }

    char *code_tag_start = find_next_code_tag_start(s);

    if (!code_tag_start) {
        splitter->section_is_code = false;
        splitter->section.data = s;
        splitter->section.len = strlen(s);
    }
    else if (code_tag_start == s) {
        splitter->section_is_code = find_next_code_tag(
            code_tag_start, &splitter->section
        );

        if (!splitter->section_is_code) {
            splitter->section.data = code_tag_start;
            splitter->section.len = strlen(code_tag_start);
        }
    }
    else {
        splitter->section_is_code = false;
        splitter->section.data = s;
        splitter->section.len = code_tag_start - s;
    }

    return true;
}

/* vi: set et ts=4 sw=4: */

