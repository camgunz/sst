#ifndef RUNE_H__
#define RUNE_H__

typedef utf8proc_int32_t rune;

bool rune_is_alpha(rune r);
bool rune_is_digit(rune r);
bool rune_is_whitespace(rune r);

#endif

/* vi: set et ts=4 sw=4: */

