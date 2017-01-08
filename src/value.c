#include <stdbool.h>

#include <mpdecimal.h>
#include <utf8proc.h>

#include "config.h"
#include "khash.h"
#include "rune.h"
#include "sslice.h"
#include "str.h"
#include "value.h"

KHASH_MAP_INIT_STR(ValueTable, Value*);

ValueStatus value_init_array(Value *value) {
    value->as.array = array_new(sizeof(Value));

    if (!value->as.array) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    return VALUE_OK;
}

ValueStatus value_init_array_count(Value *value, size_t count) {
    value->as.array = array_new_count(sizeof(Value), count);

    if (!value->as.array) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

    return VALUE_OK;
}

ValueStatus value_init_table(Value *value) {
    value->as.table = kh_init(ValueTable);

    if (!value->as.table) {
        return VALUE_DATA_MEMORY_EXHAUSTED;
    }

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
        return VALUE_DATA_MEMORY_EXHAUSTED;
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

ValueStatus value_insert(Value *value_table, const char *key, Value *value) {
    khiter_t iter;
    int rv = 0;

    if (value_table->type != VALUE_TABLE) {
        return VALUE_INVALID_TYPE;
    }

    iter = kh_put(ValueTable, value_table->as.table, key, &rv);

    if (rv == 0) {
        return VALUE_KEY_ALREADY_EXISTS;
    }

    kh_value(value_table->as.table, iter) = value;

    return VALUE_OK;
}

ValueStatus value_exists(Value *value, const char *key) {
    khiter_t iter;

    if (value->type != VALUE_TABLE) {
        return VALUE_INVALID_TYPE;
    }

    iter = kh_get(ValueTable, value->as.table, key);

    if (iter != kh_end(value->as.table)) {
        return VALUE_KEY_FOUND;
    }

    return VALUE_KEY_NOT_FOUND;
}

ValueStatus value_lookup(Value *value_table, const char *key, Value **value) {
    khiter_t iter;

    if (value_table->type != VALUE_TABLE) {
        return VALUE_INVALID_TYPE;
    }

    iter = kh_get(ValueTable, value_table->as.table, key);

    if (iter != kh_end(value_table->as.table)) {
        *value = kh_value(value_table->as.table, iter);
        return VALUE_OK;
    }

    *value = NULL;
    return VALUE_KEY_NOT_FOUND;
}

ValueStatus value_delete(Value *value, const char *key) {
    khiter_t iter;

    if (value->type != VALUE_TABLE) {
        return VALUE_INVALID_TYPE;
    }

    iter = kh_get(ValueTable, value->as.table, key);

    if (iter != kh_end(value->as.table)) {
        kh_del(ValueTable, value, iter);
        return VALUE_OK;
    }

    return VALUE_KEY_NOT_FOUND;
}

ValueStatus value_new_element_at_end(Value *value, Value **new_value) {
    void *new_element = NULL;

    if (value->type != VALUE_ARRAY) {
        return VALUE_INVALID_TYPE;
    }

    new_element = array_new_element_at_end(&value->as.array);

    if (new_element) {
        *new_value = new_element;
        return VALUE_OK;
    }

    *new_value = NULL;
    return VALUE_DATA_MEMORY_EXHAUSTED;
}

ValueStatus value_new_element_at_beginning(Value *value, Value **new_value) {
    void *new_element = NULL;

    if (value->type != VALUE_ARRAY) {
        return VALUE_INVALID_TYPE;
    }

    new_element = array_new_element_at_beginning(&value->as.array);

    if (new_element) {
        *new_value = new_element;
        return VALUE_OK;
    }

    *new_value = NULL;
    return VALUE_DATA_MEMORY_EXHAUSTED;
}

ValueStatus value_index(Value *value, size_t index, Value **element) {
    void *v = NULL;

    if (value->type != VALUE_ARRAY) {
        return VALUE_INVALID_TYPE;
    }

    if (index >= value->as.array.len) {
        *element = NULL;
        return VALUE_INDEX_OUT_OF_BOUNDS;
    }

    v = array_index(&value->as.array, index);

    if (!v) {
        *element = NULL;
        return VALUE_INDEX_OUT_OF_BOUNDS;
    }

    *element = v;
    return VALUE_OK;
}

ValueStatus value_length(Value *value, size_t *length) {
    switch (value->type) {
        case VALUE_TABLE:
            *length = kh_size(value->as.table);
            return VALUE_OK;
        case VALUE_ARRAY:
            *length = value->as.array.len;
            return VALUE_OK;
        case VALUE_STRING:
            *length = value->as.string.len;
            return VALUE_OK;
        default:
            return VALUE_INVALID_TYPE;
    }
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

ValueStatus value_clear(Value *value) {
    switch (value->type) {
        case VALUE_TABLE:
            kh_clear(value->as.table);
            break;
        case VALUE_ARRAY:
            array_clear(&value->as.array);
            break;
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

ValueStatus value_free(Value *value) {
    switch (value->type) {
        case VALUE_TABLE:
            kh_destroy(value->as.table);
            value->as.table = NULL;
            break;
        case VALUE_ARRAY:
            array_free(&value->as.array);
            break;
        case VALUE_STRING:
            sslice_clear(&value->as.string);
            break;
        case VALUE_NUMBER:
            mpd_del(value->as.number);
            value->as.number = NULL;
            break;
        case VALUE_BOOLEAN:
        default:
            break;
    }

    return VALUE_OK;
}

/* vi: set et ts=4 sw=4: */

