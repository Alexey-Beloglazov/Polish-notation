#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#include "token.h"

typedef struct {
    unsigned char *buf;
    size_t item_size;
    size_t size;
    size_t cap;
} Stack;

int stack_init(Stack *s, size_t item_size);
int stack_push(Stack *s, const void *item);
int stack_top(const Stack *s, void *out_item);
int stack_pop(Stack *s, void *out_item);
void stack_free(Stack *s);

typedef struct {
    double *a;
    size_t size;
    size_t cap;
} DStack;

int dstack_push(DStack *s, double v);
int dstack_pop(DStack *s, double *out);
void dstack_free(DStack *s);

#endif
