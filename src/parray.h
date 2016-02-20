#ifndef PARRAY_H__
#define PARRAY_H__

typedef Array PArray;

PArray* parray_new(size_t count);
bool    parray_ensure_capacity(PArray *parray, size_t count);
void    parray_rotate_left(PArray *parray);

#endif

