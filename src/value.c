#include <cbase.h>

#include "config.h"

#include "value.h"

#define unknown_type(status) status_failure( \
    status,                                  \
    "value",                                 \
    VALUE_UNKNOWN_TYPE,                      \
    "Unknown value type"                     \
)

#define invalid_type(status) status_failure( \
    status,                                  \
    "value",                                 \
    VALUE_INVALID_TYPE,                      \
    "Invalid value type"                     \
)

#define conversion_failure(status) status_failure( \
    status,                                        \
    "value",                                       \
    VALUE_CONVERSION_FAILURE,                      \
    "Conversion failure"                           \
)

static size_t key_to_hash(const void *key, size_t seed) {
    String *s = (String *)key;

    return hash64(s->data, s->byte_len, seed);
}

static void* value_to_key(const void *obj) {
    return (void *)(&((Value *)obj)->as.string);
}

static bool key_equal(const void *key1, const void *key2) {
    String *s1 = (String *)key1;
    String *s2 = (String *)key2;

    return (s1->byte_len == s2->byte_len) && (strcmp(s1->data, s2->data) == 0);
}

bool value_init(Value *value, Status *status) {
    switch (value->type) {
        case VALUE_BOOLEAN:
            value->as.boolean = false;
            return status_ok(status);
        case VALUE_NUMBER:
            decimal_from_zero(&value->as.number);
            return status_ok(status);
        case VALUE_STRING:
            string_clear(&value->as.string);
            return status_ok(status);
        case VALUE_ARRAY:
            parray_init(&value->as.array);
            return status_ok(status);
        case VALUE_TABLE:
            return table_init(
                &value->as.table,
                key_to_hash,
                value_to_key,
                key_equal,
                0,
                status
            );
        default:
            return unknown_type(status);
    }
}

bool value_init_boolean_from_sslice(Value *value, SSlice *ss, Status *status) {
    if (!value_set_type(value, VALUE_BOOLEAN, status)) {
        return false;
    }

    value->as.boolean = sslice_equals_cstr(ss, "true");

    return status_ok(status);
}

bool value_init_number_from_sslice(Value *value, SSlice *ss,
                                                 DecimalContext *ctx,
                                                 Status *status) {
    char *num = NULL;

    if (!value_set_type(value, VALUE_NUMBER, status)) {
        return false;
    }

    num = sslice_to_cstr(ss);

    if (!num) {
        return alloc_failure(status);
    }

    if (!decimal_from_cstr(&value->as.number, num, ctx, status)) {
        return false;
    }

    free(num);

    return status_ok(status);
}

bool value_init_string_from_sslice(Value *value, SSlice *ss, Status *status) {
    if (!value_set_type(value, VALUE_STRING, status)) {
        return false;
    }

    return string_init_full(
        &value->as.string,
        ss->data,
        ss->len,
        ss->byte_len,
        status
    );
}

bool value_clear(Value *value, Status *status) {
    switch (value->type) {
        case VALUE_BOOLEAN:
            value->as.boolean = false;
            break;
        case VALUE_NUMBER:
            decimal_from_zero(&value->as.number);
            break;
        case VALUE_STRING:
            string_clear(&value->as.string);
            break;
        case VALUE_ARRAY:
            parray_clear(&value->as.array);
            break;
        case VALUE_TABLE:
            if (!table_init( &value->as.table, key_to_hash, value_to_key,
                                                            key_equal,
                                                            0,
                                                            status)) {
                return false;
            }
            break;
        default:
            return unknown_type(status);
    }

    return status_ok(status);
}

bool value_set_type(Value *value, ValueType type, Status *status) {
    if (!value_clear(value, status)) {
        return false;
    }

    value->type = type;

    return status_ok(status);
}

bool value_index(Value *value, size_t index, Value **element, Status *status) {
    void *e = NULL;

    if (value->type != VALUE_ARRAY) {
        return invalid_type(status);
    }

    if (!parray_index(&value->as.array, index, &e, status)) {
        return false;
    }

    *element = e;

    return status_ok(status);
}

bool value_length(Value *value, size_t *length, Status *status) {
    switch (value->type) {
        case VALUE_STRING:
            *length = value->as.string.len;
            break;
        case VALUE_TABLE:
            *length = value->as.table.len;
            break;
        case VALUE_ARRAY:
            *length = value->as.array.len;
            break;
        default:
            return invalid_type(status);
    }

    return status_ok(status);
}

bool value_add(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status) {
    if ((op1->type != VALUE_NUMBER) || (op2->type != VALUE_NUMBER)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_NUMBER, status)) {
        return false;
    }

    if (!decimal_add(&result->as.number, &op1->as.number, &op2->as.number,
                                                          ctx,
                                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool value_sub(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status) {
    if ((op1->type != VALUE_NUMBER) || (op2->type != VALUE_NUMBER)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_NUMBER, status)) {
        return false;
    }

    if (!decimal_sub(&result->as.number, &op1->as.number, &op2->as.number,
                                                          ctx,
                                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool value_mul(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status) {
    if ((op1->type != VALUE_NUMBER) || (op2->type != VALUE_NUMBER)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_NUMBER, status)) {
        return false;
    }

    if (!decimal_mul(&result->as.number, &op1->as.number, &op2->as.number,
                                                          ctx,
                                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool value_div(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status) {
    if ((op1->type != VALUE_NUMBER) || (op2->type != VALUE_NUMBER)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_NUMBER, status)) {
        return false;
    }

    if (!decimal_div(&result->as.number, &op1->as.number, &op2->as.number,
                                                          ctx,
                                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool value_rem(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status) {
    if ((op1->type != VALUE_NUMBER) || (op2->type != VALUE_NUMBER)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_NUMBER, status)) {
        return false;
    }

    if (!decimal_rem(&result->as.number, &op1->as.number, &op2->as.number,
                                                          ctx,
                                                          status)) {
        return false;
    }

    return status_ok(status);
}

bool value_and(Value *result, Value *op1, Value *op2, Status *status) {
    if ((op1->type != VALUE_BOOLEAN) || (op2->type != VALUE_BOOLEAN)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_BOOLEAN, status)) {
        return false;
    }

    result->as.boolean = op1->as.boolean && op2->as.boolean;

    return status_ok(status);
}

bool value_or(Value *result, Value *op1, Value *op2, Status *status) {
    if ((op1->type != VALUE_BOOLEAN) || (op2->type != VALUE_BOOLEAN)) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_BOOLEAN, status)) {
        return false;
    }

    result->as.boolean = op1->as.boolean || op2->as.boolean;

    return status_ok(status);
}

bool value_not(Value *result, Value *op1, Status *status) {
    if (op1->type != VALUE_BOOLEAN) {
        return invalid_type(status);
    }

    if (!value_set_type(result, VALUE_BOOLEAN, status)) {
        return false;
    }

    result->as.boolean = !op1->as.boolean;

    return status_ok(status);
}

bool value_equal(Value *result, Value *op1, Value *op2, Status *status) {
    if (!value_set_type(result, VALUE_BOOLEAN, status)) {
        return false;
    }

    if ((op1->type == VALUE_BOOLEAN) && (op2->type == VALUE_BOOLEAN)) {
        result->as.boolean = op1->as.boolean == op2->as.boolean;
    }
    else if ((op1->type == VALUE_NUMBER) && (op2->type == VALUE_NUMBER)) {
        int cmp_res;

        if (!decimal_cmp(&op1->as.number, &op2->as.number, &cmp_res, status)) {
            return false;
        }

        result->as.boolean = cmp_res == 0;
    }
    else if ((op1->type == VALUE_STRING) && (op2->type == VALUE_STRING)) {
        result->as.boolean = (
            (op1->as.string.byte_len == op2->as.string.byte_len) &&
            (strcmp(op1->as.string.data, op2->as.string.data) == 0)
        );
    }
    else {
        return invalid_type(status);
    }

    return status_ok(status);
}

bool value_to_cstr(char **s, Value *value, Status *status) {
    char *local_s = NULL;

    switch (value->type) {
        case VALUE_NONE:
            local_s = strdup("Uninitialized value");
            break;
        case VALUE_BOOLEAN:
            if (value->as.boolean) {
                local_s = strdup("true");
            }
            else {
                local_s = strdup("false");
            }
            break;
        case VALUE_NUMBER:
            if (!decimal_to_sci_cstr(&value->as.number, false, &local_s,
                                                               status)) {
                return false;
            }
            break;
        case VALUE_STRING:
            local_s = strdup(value->as.string.data);
            break;
        default:
            return invalid_type(status);
    }

    if (!local_s) {
        return alloc_failure(status);
    }

    *s = local_s;

    return status_ok(status);
}

/* vi: set et ts=4 sw=4: */
