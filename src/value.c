#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "str.h"
#include "value.h"

ValueStatus value_init(Value *value, mpd_context_t *mpd_ctx) {
    StringStatus status;

    value->type = VALUE_NONE;
    value->mpd_ctx = mpd_ctx;

    status = string_new(&value->string, NULL);

    if (status != STRING_OK) {
        return status;
    }

    value->number = mpd_qnew();

    if (!value->number) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    return VALUE_OK;
}

ValueStatus value_clear(Value *value) {
    uint32_t res = 0;

    string_clear(value->string);
    mpd_qset_i32(value->number, 0, value->mpd_ctx, &res);

    if (res != 0) {
        return VALUE_CONVERSION_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_set_string(Value *value, String *s) {
    value_clear(value);
    value->type = VALUE_STRING;

    return string_append_len(value->string, s->data, s->len);
}

ValueStatus value_set_number(Value *value, String *n) {
    uint32_t res = 0;

    value_clear(value);
    value->type = VALUE_NUMBER;
    mpd_qset_string(value->number, n->data, value->mpd_ctx, &res);

    if (res != 0) {
        return VALUE_CONVERSION_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_add(Value *result, Value *op1, Value *op2) {
    if (op1->type == VALUE_STRING && op2->type == VALUE_STRING) {
        value_clear(result);
        result->type = VALUE_STRING;
        string_append_len(
            result->string, op1->string->data, op1->string->len
        );
        string_append_len(
            result->string, op2->string->data, op2->string->len
        );
        return VALUE_OK;
    }

    if (op1->type == VALUE_NUMBER && op2->type == VALUE_NUMBER) {
        uint32_t res = 0;

        value_clear(result);
        result->type = VALUE_NUMBER;
        mpd_qadd(
            result->number, op1->number, op2->number, result->mpd_ctx, &res
        );

        if (res != 0) {
            return VALUE_ARITHMETIC_ERROR;
        }

        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}

ValueStatus value_sub(Value *result, Value *op1, Value *op2) {
    uint32_t res = 0;

    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;
    mpd_qsub(
        result->number, op1->number, op2->number, result->mpd_ctx, &res
    );

    if (res != 0) {
        return VALUE_ARITHMETIC_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_mul(Value *result, Value *op1, Value *op2) {
    uint32_t res = 0;

    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpd_qmul(
        result->number, op1->number, op2->number, result->mpd_ctx, &res
    );

    if (res != 0) {
        return VALUE_ARITHMETIC_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_div(Value *result, Value *op1, Value *op2) {
    uint32_t res = 0;

    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpd_qdiv(
        result->number, op1->number, op2->number, result->mpd_ctx, &res
    );

    if (res != 0) {
        return VALUE_ARITHMETIC_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_rem(Value *result, Value *op1, Value *op2) {
    uint32_t res = 0;

    if (op1->type != VALUE_NUMBER || op2->type != VALUE_NUMBER) {
        return VALUE_INVALID_TYPE;
    }

    value_clear(result);
    result->type = VALUE_NUMBER;

    mpd_qrem(
        result->number, op1->number, op2->number, result->mpd_ctx, &res
    );

    if (res != 0) {
        return VALUE_ARITHMETIC_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_as_string(char **s, Value *value) {
    if (value->type == VALUE_NUMBER) {
        *s = mpd_to_sci(value->number, 0);

        if (!s) {
            return VALUE_CONVERSION_ERROR;
        }
    }
    else {
        *s = value->string->data;
    }

    return VALUE_OK;
}

/* vi: set et ts=4 sw=4: */

