typedef struct {
    size_t  esize;
    size_t  ecount;
    void   *elements;
} Array;

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

    return array;
}

bool array_ensure_capacity(Array *array, size_t count) {
    if (array->count >= count) {
        return true;
    }

    void *new_elements = realloc(array->elements, array->esize * count);

    if (!new_elements) {
        return false;
    }

    array->elements = new_elements;
    array->count = count;

    return true;
}

