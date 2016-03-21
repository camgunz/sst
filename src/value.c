#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "rune.h"
#include "sslice.h"
#include "str.h"
#include "value.h"

ValueStatus value_clear(Value *value) {
    switch (value->type) {
        case VALUE_STRING:
            sslice_clear(&value->as.string);
            break;
        case VALUE_NUMBER:
            mpd_zerocoeff(value->as.number);
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

ValueStatus value_init_number(Value *value, mpd_context_t *mpd_ctx) {
    value->as.number = mpd_qnew();

    if (!value->as.number) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    mpd_zerocoeff(value->as.number);

    value->type = VALUE_NUMBER;
    value->mpd_ctx = mpd_ctx;

    return VALUE_OK;
}

ValueStatus value_clear_number(Value *value, mpd_context_t *mpd_ctx) {
    value_clear(value);
    value_init_number(value, mpd_ctx);

    return VALUE_OK;
}

ValueStatus value_set_number(Value *value, mpd_t *n) {
    int res = 0;
    uint32_t status = 0;

    res = mpd_qcopy(value->as.number, n, &status);

    if (res != 1) {
        return VALUE_DATA_MEMORY_EXHAUSTED;;
    }

    value->type = VALUE_NUMBER;

    return VALUE_OK;
}

ValueStatus value_set_number_from_sslice(Value *value, SSlice *s) {
    uint32_t res = 0;
    char *number_as_string = sslice_to_c_string(s);

    value->as.number = mpd_qnew();

    if (!value->as.number) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    number_as_string = sslice_to_c_string(s);

    if (!number_as_string) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    mpd_qset_string(value->as.number, number_as_string, value->mpd_ctx, &res);

    free(number_as_string);

    if (res != 0) {
        return VALUE_CONVERSION_ERROR;
    }

    value->type = VALUE_NUMBER;

    return VALUE_OK;
}

ValueStatus value_set_boolean(Value *value, bool b) {
    value->type = VALUE_BOOLEAN;
    value->as.boolean = b;

    return VALUE_OK;
}

ValueStatus value_add(Value *result, Value *op1, Value *op2) {
#if 0
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
#endif

    if (op1->type == VALUE_NUMBER && op2->type == VALUE_NUMBER) {
        uint32_t res = 0;

        value_clear_number(result, op1->mpd_ctx);

        mpd_qadd(
            result->as.number,
            op1->as.number,
            op2->as.number,
            result->mpd_ctx,
            &res
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

    value_clear_number(result, op1->mpd_ctx);

    mpd_qsub(
        result->as.number,
        op1->as.number,
        op2->as.number,
        result->mpd_ctx,
        &res
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

    value_clear_number(result, op1->mpd_ctx);

    mpd_qmul(
        result->as.number,
        op1->as.number,
        op2->as.number,
        result->mpd_ctx,
        &res
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

    value_clear_number(result, op1->mpd_ctx);

    mpd_qdiv(
        result->as.number,
        op1->as.number,
        op2->as.number,
        result->mpd_ctx,
        &res
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

    value_clear_number(result, op1->mpd_ctx);

    mpd_qrem(
        result->as.number,
        op1->as.number,
        op2->as.number,
        result->mpd_ctx,
        &res
    );

    if (res != 0) {
        return VALUE_ARITHMETIC_ERROR;
    }

    return VALUE_OK;
}

ValueStatus value_and(Value *result, Value *op1, Value *op2) {
    if (op1->type == VALUE_BOOLEAN && op2->type == VALUE_BOOLEAN) {
        value_clear(result);
        value_set_boolean(result, op1->as.boolean && op2->as.boolean);

        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}

ValueStatus value_or(Value *result, Value *op1, Value *op2) {
    if (op1->type == VALUE_BOOLEAN && op2->type == VALUE_BOOLEAN) {
        value_clear(result);
        value_set_boolean(result, op1->as.boolean || op2->as.boolean);

        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}

ValueStatus value_xor(Value *result, Value *op1, Value *op2) {
    if (op1->type == VALUE_BOOLEAN && op2->type == VALUE_BOOLEAN) {
        value_clear(result);

        if (op1->as.boolean && !op2->as.boolean) {
            value_set_boolean(result, true);
        }
        else if (op2->as.boolean && !op1->as.boolean) {
            value_set_boolean(result, true);
        }
        else {
            value_set_boolean(result, false);
        }

        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}

ValueStatus value_not(Value *result, Value *op) {
    if (op->type == VALUE_BOOLEAN) {
        value_clear(result);
        value_set_boolean(result, !op->as.boolean);

        return VALUE_OK;
    }

    return VALUE_INVALID_TYPE;
}


ValueStatus value_as_string(char **s, Value *value) {
    char *vstring;

    switch (value->type) {
        case VALUE_STRING:
            *s = sslice_to_c_string(&value->as.string);
            break;
        case VALUE_NUMBER:
            vstring = mpd_to_sci(value->as.number, 0);

            if (!s) {
                return VALUE_CONVERSION_ERROR;
            }

            *s = vstring;
            break;
        case VALUE_BOOLEAN:
            switch (value->as.boolean) {
                case true:
                    *s = strdup("true");
                case false:
                    *s = strdup("false");
                default:
                    return VALUE_CONVERSION_ERROR;
            }
            break;
        case VALUE_NONE:
            *s = strdup("Uninitialized value");
            break;
    }

    return VALUE_OK;
}

/* vi: set et ts=4 sw=4: */

