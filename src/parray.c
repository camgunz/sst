#include <stdbool.h>
#include <stdlib.h>

#include "array.h"
#include "parray.h"

PArray* parray_new(size_t count) {
    return array_new(sizeof(void *), count);
}

bool parray_ensure_capacity(PArray *parray, size_t count) {
    return array_ensure_capacity(parray, count);
}

void parray_rotate_left(PArray *parray) {
    void *first_element = parray->elements[0];

    memmove(
        &parray->elements[1],
        &parray->elements[0],
        sizeof(void *) * (parray->count - 2)
    );

    parray->elements[parray->count - 1] = first_element;
}

