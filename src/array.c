#include <stdbool.h>
#include <stdlib.h>

#include "array.h"

Array* array_new(size_t size, size_t count) {
    Array *array = malloc(sizeof(Array));

    if (!array) {
        return NULL;
    }

    array->elements = malloc(size * count);

    if (!array->elements) {
        free(array);
        return NULL;
    }

    array->esize = size;
    array->ecount = count;

    return array;
}

bool array_ensure_capacity(Array *array, size_t count) {
    if (array->ecount >= count) {
        return true;
    }

    void *new_elements = realloc(array->elements, array->esize * count);

    if (!new_elements) {
        return false;
    }

    array->elements = new_elements;
    array->ecount = count;

    return true;
}

