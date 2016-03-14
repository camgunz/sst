#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "str.h"
#include "value.h"

ValueStatus value_clear(Value *value) {
    uint32_t res = 0;

    switch (value->type) {
        case VALUE_STRING:
            sslice_clear(value->as.string);
            break;
        case VALUE_NUMBER:
            mpd_del(value->as.number);
            value->mpd_ctx = NULL;
            break;
        case VALUE_BOOLEAN:
        default:
            break;
    }

    value->type = VALUE_NONE;

    return VALUE_OK;
}

ValueStatus value_set_string(Value *value, SSlice *s) {
    value_clear(value);

    value->type = VALUE_STRING;
    sslice_shallow_copy(&value->as.string, s);

    return VALUE_OK;
}

ValueStatus value_set_number(Value *value, mpd_t *n, mpd_context_t *mpd_ctx) {
    uint32_t res = 0;

    res = mpd_qcopy(value->as.number, n);

    if (res != 1) {
        return VALUE_DATA_MEMORY_EXHAUSTED;;
    }

    value->mpd_ctx = mpd_ctx;

    return VALUE_OK;
}

ValueStatus value_set_number_from_sslice(Value *value, SSlice *s,
                                         mpd_context_t *mpd_ctx) {
    ValueStatus vstatus;
    uint32_t res = 0;
    char *number_as_string = sslice_to_c_string(s);

    value->as.number = mpd_qnew();

    if (!value->as.string) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    number_as_string = sslice_to_c_string(s);

    if (!number_as_string) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    mpd_qset_string(value->as.number, n->data, value->mpd_ctx, &res);

    free(number_as_string);

    if (res != 0) {
        return VALUE_CONVERSION_ERROR;
    }

    value->type = VALUE_NUMBER;
    value->mpd_ctx = mpd_ctx;

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
    switch (value->type) {
        case VALUE_STRING:
            *s = value->as.string->data;
            break;
        case VALUE_NUMBER:
            *s = mpd_to_sci(value->as.number, 0);

            if (!s) {
                return VALUE_CONVERSION_ERROR;
            }
            break;
        case VALUE_BOOLEAN:
            switch (value->as.boolean) {
                case true:
                    *s = "true";
                case false:
                    *s = "false";
                default:
                    return VALUE_CONVERSION_ERROR;
            }
    }

    return VALUE_OK;
}

/* vi: set et ts=4 sw=4: */

