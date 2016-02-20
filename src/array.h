#ifndef ARRAY_H__
#define ARRAY_H__

typedef struct {
    size_t  esize;
    size_t  ecount;
    void   *elements;
} Array;

Array* array_new(size_t size, size_t count);
bool   array_ensure_capacity(Array *array, size_t count);

#endif

/* vi: set et ts=4 sw=4: */

