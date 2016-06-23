#ifndef STACK_H__
#define STACK_H__

typedef struct {
    size_t  element_size;
    size_t  depth;
    size_t  cursor;
    void   *elements;
} Stack;

Stack* stack_new(size_t element_size);
Stack* stack_new_depth(size_t element_size, size_t depth);
bool   stack_ensure_capacity(Stack *stack, size_t depth);
void*  stack_head(Stack *stack);
void*  stack_push(Stack *stack);
void*  stack_pop(Stack *stack);
void   stack_clear(Stack *stack);
void   stack_free(Stack *stack);

#endif

/* vi: set et ts=4 sw=4: */

