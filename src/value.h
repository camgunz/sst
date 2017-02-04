#ifndef VALUE_H__
#define VALUE_H__

enum {
    VALUE_UNKNOWN_TYPE = 1,
    VALUE_INVALID_TYPE,
    VALUE_CONVERSION_FAILURE,
    VALUE_INVALID_BOOLEAN_VALUE,
    VALUE_INVALID_FUNCTION_ARGUMENT_TYPE,
    VALUE_MISMATCHED_FUNCTION_ARITY_AND_TYPES,
};

typedef enum {
    VALUE_NONE,
    VALUE_BOOLEAN,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_TABLE,
    VALUE_FUNCTION,
} ValueType;

typedef struct {
    unsigned int arity;
    const char *argument_types;
} Function;

typedef struct {
    ValueType type;
    size_t decimal_data[DECIMAL_MINALLOC_MAX];
    union {
        String string;
        Decimal number;
        bool boolean;
        PArray array;
        Table table;
        Function function;
    } as;
} Value;

void value_init_boolean(Value *value, bool b);
bool value_init_number(Value *value, const char *num, DecimalContext *ctx,
                                                      Status *status);
bool value_init_string(Value *value, const char *string, Status *status);
void value_init_array(Value *value);
bool value_init_table(Value *value, Status *status);
bool value_init_function(Value *value, unsigned int arity,
                                       const char *argument_types,
                                       Status *status);

bool value_init_boolean_from_sslice(Value *value, SSlice *ss, Status *status);
bool value_init_number_from_sslice(Value *value, SSlice *ss,
                                                 DecimalContext *ctx,
                                                 Status *status);
bool value_init_string_from_sslice(Value *value, SSlice *ss, Status *status);
void value_clear(Value *value);
bool value_set_boolean(Value *value, bool b, Status *status);
bool value_set_number(Value *value, Decimal *n, Status *status);
bool value_set_string(Value *value, String *s, Status *status);
bool value_set_array(Value *value, PArray *parray, Status *status);
bool value_set_table(Value *value, Table *table, Status *status);

bool value_index(Value *value, size_t index, Value **element, Status *status);
bool value_length(Value *value, size_t *length, Status *status);

bool value_add(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_sub(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_mul(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_div(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_rem(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_pow(Value *result, Value *op1, Value *op2, DecimalContext *ctx,
                                                      Status *status);
bool value_and(Value *result, Value *op1, Value *op2, Status *status);
bool value_or(Value *result, Value *op1, Value *op2, Status *status);
bool value_not(Value *result, Value *op1, Status *status);
bool value_equal(Value *result, Value *op1, Value *op2, Status *status);
bool value_to_cstr(char **s, Value *value, Status *status);
void value_free(Value *value);

static inline
void value_set_type(Value *value, ValueType type) {
    if (value->type != type) {
        value_free(value);
    }

    value->type = type;
}

#endif

/* vi: set et ts=4 sw=4: */
