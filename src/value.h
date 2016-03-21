#ifndef VALUE_H__
#define VALUE_H__

typedef enum {
    VALUE_NONE,
    VALUE_STRING,
    VALUE_NUMBER,
    VALUE_BOOLEAN,
} ValueType;

typedef enum {
    VALUE_OK,
    VALUE_DATA_MEMORY_EXHAUSTED = STRING_DATA_MEMORY_EXHAUSTED,
    VALUE_INVALID_TYPE,
    VALUE_CONVERSION_ERROR,
    VALUE_ARITHMETIC_ERROR,
    VALUE_NULL_CONTEXT_ERROR,
    VALUE_MAX
} ValueStatus;

typedef struct {
    ValueType      type;
    mpd_context_t *mpd_ctx;
    union {
        SSlice  string;
        mpd_t  *number;
        bool    boolean;
    } as;
} Value;

ValueStatus value_clear(Value *value);
ValueStatus value_set_string(Value *value, SSlice *s);
ValueStatus value_init_number(Value *value, mpd_context_t *mpd_ctx);
ValueStatus value_clear_number(Value *value, mpd_context_t *mpd_ctx);
ValueStatus value_set_number(Value *value, mpd_t *n);
ValueStatus value_set_number_from_sslice(Value *value, SSlice *s);
ValueStatus value_set_boolean(Value *value, bool b);
ValueStatus value_add(Value *result, Value *op1, Value *op2);
ValueStatus value_sub(Value *result, Value *op1, Value *op2);
ValueStatus value_mul(Value *result, Value *op1, Value *op2);
ValueStatus value_div(Value *result, Value *op1, Value *op2);
ValueStatus value_rem(Value *result, Value *op1, Value *op2);
ValueStatus value_and(Value *result, Value *op1, Value *op2);
ValueStatus value_or(Value *result, Value *op1, Value *op2);
ValueStatus value_xor(Value *result, Value *op1, Value *op2);
ValueStatus value_not(Value *result, Value *op);
ValueStatus value_as_string(char **s, Value *value);

#endif

/* vi: set et ts=4 sw=4: */

