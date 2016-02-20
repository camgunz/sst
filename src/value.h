#ifndef VALUE_H__
#define VALUE_H__

typedef enum {
    VALUE_NONE,
    VALUE_STRING,
    VALUE_NUMBER
} ValueType;

typedef enum {
    VALUE_OK,
    VALUE_DATA_MEMORY_EXHAUSTED = STRING_DATA_MEMORY_EXHAUSTED,
    VALUE_INVALID_TYPE,
    VALUE_CONVERSION_ERROR,
    VALUE_ARITHMETIC_ERROR,
    VALUE_MAX
} ValueStatus;

typedef struct {
    mpd_context_t *mpd_ctx;
    ValueType      type;
    String        *string;
    mpd_t         *number;
} Value;

ValueStatus value_init(Value *value, mpd_context_t *mpd_ctx);
ValueStatus value_clear(Value *value);
ValueStatus value_set_string(Value *value, String *s);
ValueStatus value_set_number(Value *value, String *n);
ValueStatus value_add(Value *result, Value *op1, Value *op2);
ValueStatus value_sub(Value *result, Value *op1, Value *op2);
ValueStatus value_mul(Value *result, Value *op1, Value *op2);
ValueStatus value_div(Value *result, Value *op1, Value *op2);
ValueStatus value_rem(Value *result, Value *op1, Value *op2);
ValueStatus value_as_string(char **s, Value *value);

#endif

/* vi: set et ts=4 sw=4: */

