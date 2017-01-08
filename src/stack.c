#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "stack.h"

Stack* stack_new(size_t element_size) {
    Stack *stack = malloc(sizeof(Stack));

    if (!stack) {
        return NULL;
    }

    stack->elements     = NULL;
    stack->element_size = element_size;
    stack->cursor       = 0;
    stack->depth        = 0;

    return stack;
}

Stack *stack_new_depth(size_t element_size, size_t depth) {
    Stack *stack = malloc(sizeof(Stack));

    if (!stack) {
        return NULL;
    }

    if (depth > 0) {
        stack->elements = malloc(element_size * depth);

        if (!stack->elements) {
            free(stack);
            return NULL;
        }
    }

    stack->element_size = element_size;
    stack->depth        = depth;

    return stack;
}

bool stack_ensure_capacity(Stack *stack, size_t depth) {
    void *elements = NULL;

    if (stack->depth >= depth) {
        return true;
    }

    elements = realloc(stack->elements, depth * stack->element_size);

    if (!elements) {
        return false;
    }

    stack->elements = elements;
    stack->depth    = depth;

    return true;
}

void *stack_head(Stack *stack) {
    return ((char *)stack->elements) + (
        (stack->cursor - 1) * stack->element_size
    );
}

void* stack_push(Stack *stack) {
    if (!stack_ensure_capacity(stack, stack->depth + 1)) {
        return NULL;
    }

    stack->cursor++;

    return stack_head(stack);
}

void *stack_pop(Stack *stack) {
    void *element = NULL;
    
    if (stack->cursor == 0) {
        return NULL;
    }
    
    element = stack_head(stack);

    stack->cursor--;

    return element;
}

void stack_clear(Stack *stack) {
    stack->cursor = 0;
}

void stack_free(Stack *stack) {
    free(stack->elements);
    stack->element_size  = 0;
    stack->depth         = 0;
    stack->cursor        = 0;
    stack->elements      = NULL;
}

/* vi: set et ts=4 sw=4: */

