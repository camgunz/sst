#ifndef ARRAY_H__
#define ARRAY_H__

typedef struct {
    size_t  element_size;
    size_t  len;
    void   *elements;
} Array;

Array* array_new(size_t element_size);
Array* array_new_length(size_t element_size, size_t length);
bool   array_ensure_capacity(Array *array, size_t length);
void*  array_new_element_at_end(Array *array);
void*  array_new_element_at_beginning(Array *array);
void*  array_index(Array *array, size_t index);
void   array_clear(Array *array);
void   array_free(Array *array);

#endif

/* vi: set et ts=4 sw=4: */

