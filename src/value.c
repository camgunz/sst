#include <stdbool.h>

#include <glib.h>
#include <gmp.h>
#include <mpfr.h>

#include "config.h"
#include "str.h"
#include "value.h"

void value_init(Value *value) {
    value->type = VALUE_NONE;
    value->string = g_string_new(NULL);
    mpfr_init2(value->number, DEFAULT_PRECISION);
}

void value_clear(Value *value) {
    g_string_erase(value->string, 0, -1);
    mpfr_set_ui(value->number, 0, MPFR_RNDZ);
    // mpfr_clear(value->number);
}

void value_set_string(Value *value, String *s) {
    value_clear(value);
    value->type = VALUE_STRING;
    g_string_append_len(value->string, s->data, s->len);
}

void value_set_number(Value *value, String *n) {
    value_clear(value);
    value->type = VALUE_NUMBER;
    mpfr_strtofr(value->number, n->data, NULL, 0, MPFR_RNDZ);
}

ValueStatus value_add(Value *result, Value *op1, Value *op2) {
    if (op1->type == VALUE_STRING && op2->type == VALUE_STRING) {
        value_clear(result);
        result->type = VALUE_STRING;
        g_string_append_len(
            result->string, op1->string->str, op1->string->len
        );
        g_string_append_len(
            result->string, op2->string->str, op2->string->len
        );
        return VALUE_OK;
    }

    if (op1->type == VALUE_NUMBER && op2->type == VALUE_NUMBER) {
        value_clear(result);
        result->type = VALUE_NUMBER;
        mpfr_add(result->number, op1->number, op2->number, MPFR_RNDZ);
        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}

ValueStatus value_sub(Value *result, Value *op1, Value *op2) {
    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;
    mpfr_sub(result->number, op1->number, op2->number, MPFR_RNDZ);
    return VALUE_OK;
}

ValueStatus value_mul(Value *result, Value *op1, Value *op2) {
    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpfr_mul(result->number, op1->number, op2->number, MPFR_RNDZ);
    return VALUE_OK;
}

ValueStatus value_div(Value *result, Value *op1, Value *op2) {
    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpfr_div(result->number, op1->number, op2->number, MPFR_RNDZ);
    return VALUE_OK;
}

ValueStatus value_rem(Value *result, Value *op1, Value *op2) {
    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpfr_fmod(result->number, op1->number, op2->number, MPFR_RNDZ);
    return VALUE_OK;
}

ValueStatus value_as_string(char **s, Value *value) {
    if (value->type == VALUE_NUMBER) {
        mpfr_exp_t ep;
        char *s = mpfr_get_str(NULL, &ep, 10, 0, value->number, MPFR_RNDZ);

        if (!s) {
            return VALUE_CONVERSION_ERROR;
        }

        g_string_assign(value->string, s);
    }

    *s = value->string->str;
    return VALUE_OK;
}

/* vi: set et ts=4 sw=4: */

