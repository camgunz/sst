#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "array.h"

Array* array_new(size_t element_size) {
    Array *array = malloc(sizeof(Array));

    if (!array) {
        return NULL;
    }

    array->elements     = NULL;
    array->element_size = element_size;
    array->len          = 0;

    return array;
}

Array* array_new_length(size_t element_size, size_t length) {
    Array *array = malloc(sizeof(Array));

    if (!array) {
        return NULL;
    }

    if (length > 0) {
        array->elements = malloc(element_size * length);

        if (!array->elements) {
            free(array);
            return NULL;
        }
    }

    array->element_size = element_size;
    array->len          = length;

    return array;
}

bool array_ensure_capacity(Array *array, size_t length) {
    void *new_elements = NULL;

    if (array->len >= length) {
        return true;
    }

    new_elements = realloc(array->elements, length * array->element_size);

    if (!new_elements) {
        return false;
    }

    array->elements = new_elements;
    array->len      = length;

    return true;
}

void* array_new_element_at_end(Array *array) {
    if (!array_ensure_capacity(array, array->len + 1)) {
        return NULL;
    }

    return array_index(array, array->len - 1);
}

void* array_new_element_at_beginning(Array *array) {
    char *elements = NULL;

    if (!array_ensure_capacity(array, array->len + 1)) {
        return NULL;
    }

    elements = (char *)array->elements;

    memmove(
        elements,
        elements + array->element_size,
        (array->len - 1) * array->element_size
    );

    memset(elements, 0, array->element_size);

    return array_index(array, 0);
}

void* array_index(Array *array, size_t index) {
    if (index >= array->len) {
        return NULL;
    }

    return ((char *)array->elements) + (index * array->element_size);
}

void array_clear(Array *array) {
    memset(array->elements, 0, array->len * array->element_size);
}

void array_free(Array *array) {
    free(array->elements);
    array->element_size  = 0;
    array->len           = 0;
    array->elements      = NULL;
}

/* vi: set et ts=4 sw=4: */

