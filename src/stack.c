#include "stack.h"

#include <stdlib.h>
#include <string.h>

int stack_init(Stack *s, size_t item_size) {
    s->buf = NULL;
    s->item_size = item_size;
    s->size = 0;
    s->cap = 0;
    return 0;
}

static int stack_reserve(Stack *s, size_t need) {
    if (need <= s->cap) return 0;
    size_t nc = s->cap ? s->cap * 2 : 16;
    while (nc < need) nc *= 2;
    unsigned char *nb = (unsigned char *)realloc(s->buf, nc * s->item_size);
    if (!nb) return -1;
    s->buf = nb;
    s->cap = nc;
    return 0;
}

int stack_push(Stack *s, const void *item) {
    if (stack_reserve(s, s->size + 1) != 0) return -1;
    memcpy(s->buf + s->size * s->item_size, item, s->item_size);
    s->size += 1;
    return 0;
}

int stack_top(const Stack *s, void *out_item) {
    if (s->size == 0) return -1;
    memcpy(out_item, s->buf + (s->size - 1) * s->item_size, s->item_size);
    return 0;
}

int stack_pop(Stack *s, void *out_item) {
    if (s->size == 0) return -1;
    s->size -= 1;
    if (out_item) memcpy(out_item, s->buf + s->size * s->item_size, s->item_size);
    return 0;
}

void stack_free(Stack *s) {
    free(s->buf);
    s->buf = NULL;
    s->size = s->cap = 0;
    s->item_size = 0;
}

static int dstack_reserve(DStack *s, size_t need) {
    if (need <= s->cap) return 0;
    size_t nc = s->cap ? s->cap * 2 : 16;
    while (nc < need) nc *= 2;
    double *na = (double *)realloc(s->a, nc * sizeof(double));
    if (!na) return -1;
    s->a = na;
    s->cap = nc;
    return 0;
}

int dstack_push(DStack *s, double v) {
    if (dstack_reserve(s, s->size + 1) != 0) return -1;
    s->a[s->size++] = v;
    return 0;
}

int dstack_pop(DStack *s, double *out) {
    if (s->size == 0) return -1;
    s->size -= 1;
    if (out) *out = s->a[s->size];
    return 0;
}

void dstack_free(DStack *s) {
    free(s->a);
    s->a = NULL;
    s->size = s->cap = 0;
}
