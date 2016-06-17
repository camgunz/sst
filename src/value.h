#ifndef VALUE_H__
#define VALUE_H__

typedef enum {
    VALUE_NONE,
    VALUE_TABLE,
    VALUE_ARRAY,
    VALUE_STRING,
    VALUE_NUMBER,
    VALUE_BOOLEAN
} ValueType;

typedef enum {
    VALUE_OK,
    VALUE_DATA_MEMORY_EXHAUSTED = STRING_DATA_MEMORY_EXHAUSTED,
    VALUE_INVALID_TYPE,
    VALUE_CONVERSION_ERROR,
    VALUE_ARITHMETIC_ERROR,
    VALUE_NULL_CONTEXT_ERROR,
    VALUE_KEY_FOUND,
    VALUE_KEY_NOT_FOUND,
    VALUE_INDEX_OUT_OF_BOUNDS,
    VALUE_MAX
} ValueStatus;

#define VALUE_KEY_ALREADY_EXISTS VALUE_KEY_FOUND

typedef struct {
    ValueType      type;
    mpd_context_t *mpd_ctx;
    union {
        SSlice  string;
        mpd_t  *number;
        bool    boolean;
        Array   array;
        void   *table;
    } as;
} Value;

ValueStatus value_init_array(Value *value);
ValueStatus value_init_array_count(Value *value);
ValueStatus value_init_table(Value *value);
ValueStatus value_set_string(Value *value, SSlice *s);
ValueStatus value_init_number(Value *value, mpd_context_t *mpd_ctx);
ValueStatus value_clear_number(Value *value, mpd_context_t *mpd_ctx);
ValueStatus value_set_number(Value *value, mpd_t *n);
ValueStatus value_set_number_from_sslice(Value *value, SSlice *s);
ValueStatus value_set_boolean(Value *value, bool b);
ValueStatus value_insert(Value *value_table, const char *key, Value *value);
ValueStatus value_exists(Value *value, const char *key);
ValueStatus value_lookup(Value *value_table, const char *key, Value **value);
ValueStatus value_delete(Value *value, const char *key);
ValueStatus value_new_element_at_end(Value *value, Value **new_value);
ValueStatus value_new_element_at_beginning(Value *value, Value **new_value);
ValueStatus value_index(Value *value, size_t index, Value **element);
ValueStatus value_length(Value *value, size_t *length);
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
ValueStatus value_clear(Value *value);
ValueStatus value_free(Value *value);

#endif

/* vi: set et ts=4 sw=4: */

