#ifndef VALUE_H__
#define VALUE_H__

typedef enum {
    VALUE_NONE,
    VALUE_STRING,
    VALUE_NUMBER
} ValueType;

typedef enum {
    VALUE_OK,
    VALUE_INVALID_TYPE,
    VALUE_CONVERSION_ERROR,
    VALUE_MAX
} ValueStatus;

typedef struct {
    ValueType  type;
    GString   *string;
    mpfr_t     number;
} Value;

void        value_init(Value *value);
void        value_clear(Value *value);
void        value_set_string(Value *value, String *s);
void        value_set_number(Value *value, String *n);
ValueStatus value_add(Value *result, Value *op1, Value *op2);
ValueStatus value_sub(Value *result, Value *op1, Value *op2);
ValueStatus value_mul(Value *result, Value *op1, Value *op2);
ValueStatus value_div(Value *result, Value *op1, Value *op2);
ValueStatus value_rem(Value *result, Value *op1, Value *op2);
ValueStatus value_as_string(char **s, Value *value);

#endif

/* vi: set et ts=4 sw=4: */

