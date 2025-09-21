#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "stack.h"
#include "token.h"

static int prec_op(OpKind op) {
    int p = -1;
    if (op == OP_ADD || op == OP_SUB)
        p = 1;
    else if (op == OP_MUL || op == OP_DIV)
        p = 2;
    else if (op == OP_POW)
        p = 3;
    return p;
}
static int right_assoc(OpKind op) { return (op == OP_POW); }

static int rpn_push(TokenVec *v, Token t) {
    int rc = 0;
    if (v->size == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 16;
        Token *na = (Token *)realloc(v->data, nc * sizeof(Token));
        if (!na)
            rc = -1;
        else {
            v->data = na;
            v->cap = nc;
        }
    }
    if (rc == 0) v->data[v->size++] = t;
    return rc;
}

static int pop_for_operator(Stack *st, Token cur, TokenVec *out) {
    int rc = 0, loop = 1;
    while (loop == 1 && rc == 0) {
        if (st->size == 0)
            loop = 0;
        else {
            Token top;
            if (stack_top(st, &top) != 0) rc = -1;
            if (rc == 0) {
                if (top.type == TK_OPERATOR) {
                    int p1 = prec_op(top.op), p2 = prec_op(cur.op);
                    int need = (p1 > p2) || (p1 == p2 && !right_assoc(cur.op));
                    if (need) {
                        if (stack_pop(st, &top) != 0) rc = -1;
                        if (rc == 0 && rpn_push(out, top) != 0) rc = -1;
                    } else
                        loop = 0;
                } else if (top.type == TK_FUNCTION) {
                    if (stack_pop(st, &top) != 0) rc = -1;
                    if (rc == 0 && rpn_push(out, top) != 0) rc = -1;
                } else
                    loop = 0;
            }
        }
    }
    return rc;
}

static int handle_rparen(Stack *st, TokenVec *out) {
    int rc = 0, found = 0;
    while (st->size > 0 && rc == 0 && found == 0) {
        Token top;
        if (stack_pop(st, &top) != 0) rc = -1;
        if (rc == 0) {
            if (top.type == TK_LPAREN)
                found = 1;
            else if (rpn_push(out, top) != 0)
                rc = -1;
        }
    }
    if (rc == 0) {
        if (!found)
            rc = -1;
        else if (st->size > 0) {
            Token top2;
            if (stack_top(st, &top2) == 0 && top2.type == TK_FUNCTION) {
                if (stack_pop(st, &top2) != 0) rc = -1;
                if (rc == 0 && rpn_push(out, top2) != 0) rc = -1;
            }
        }
    }
    return rc;
}

int to_rpn(const Token *in, size_t n, TokenVec *out_rpn) {
    int rc = 0;
    size_t i = 0;
    Stack st;
    out_rpn->data = NULL;
    out_rpn->size = out_rpn->cap = 0;
    stack_init(&st, sizeof(Token));

    while (i < n && rc == 0) {
        Token cur = in[i];
        if (cur.type == TK_NUMBER || cur.type == TK_VARIABLE)
            rc = rpn_push(out_rpn, cur);
        else if (cur.type == TK_FUNCTION || cur.type == TK_LPAREN)
            rc = stack_push(&st, &cur);
        else if (cur.type == TK_OPERATOR) {
            if (pop_for_operator(&st, cur, out_rpn) == 0)
                rc = stack_push(&st, &cur);
            else
                rc = -1;
        } else if (cur.type == TK_RPAREN)
            rc = handle_rparen(&st, out_rpn);
        else
            rc = -1;
        if (rc == 0) i++;
    }

    while (st.size > 0 && rc == 0) {
        Token top;
        if (stack_pop(&st, &top) != 0)
            rc = -1;
        else if (top.type == TK_LPAREN || top.type == TK_RPAREN)
            rc = -1;
        else if (rpn_push(out_rpn, top) != 0)
            rc = -1;
    }
    stack_free(&st);
    if (rc != 0) {
        free(out_rpn->data);
        out_rpn->data = NULL;
        out_rpn->size = out_rpn->cap = 0;
    }
    return (rc == 0) ? 0 : -1;
}

static int apply_op(OpKind op, double a, double b, double *out) {
    int rc = 0;
    if (op == OP_ADD)
        *out = a + b;
    else if (op == OP_SUB)
        *out = a - b;
    else if (op == OP_MUL)
        *out = a * b;
    else if (op == OP_DIV) {
        if (b == 0.0)
            rc = -1;
        else
            *out = a / b;
    } else if (op == OP_POW)
        *out = pow(a, b);
    else
        rc = -1;
    return rc;
}
static int apply_fn(FnKind fn, double v, double *out) {
    int rc = 0;
    if (fn == FN_NEG)
        *out = -v;
    else if (fn == FN_SIN)
        *out = sin(v);
    else if (fn == FN_COS)
        *out = cos(v);
    else if (fn == FN_TAN)
        *out = tan(v);
    else if (fn == FN_CTG) {
        double t = tan(v);
        if (fabs(t) < 1e-12)
            rc = -1;
        else
            *out = 1.0 / t;
    } else if (fn == FN_SQRT) {
        if (v < 0.0)
            rc = -1;
        else
            *out = sqrt(v);
    } else if (fn == FN_LN) {
        if (v <= 0.0)
            rc = -1;
        else
            *out = log(v);
    } else
        rc = -1;
    return rc;
}

int eval_rpn(const Token *rpn, size_t n, double x_value, double *out_y) {
    int rc = 0;
    size_t i = 0;
    DStack st = {0};
    while (i < n && rc == 0) {
        Token t = rpn[i];
        double r = 0.0;
        if (t.type == TK_NUMBER)
            rc = dstack_push(&st, t.number);
        else if (t.type == TK_VARIABLE)
            rc = dstack_push(&st, x_value);
        else if (t.type == TK_FUNCTION) {
            double a = 0.0;
            if (dstack_pop(&st, &a) != 0) rc = -1;
            if (rc == 0 && apply_fn(t.fn, a, &r) != 0) rc = -1;
            if (rc == 0 && ((isnan(r) || isinf(r)) || dstack_push(&st, r) != 0)) rc = -1;
        } else if (t.type == TK_OPERATOR) {
            double b = 0.0, a = 0.0;
            if (dstack_pop(&st, &b) != 0 || dstack_pop(&st, &a) != 0) rc = -1;
            if (rc == 0 && apply_op(t.op, a, b, &r) != 0) rc = -1;
            if (rc == 0 && ((isnan(r) || isinf(r)) || dstack_push(&st, r) != 0)) rc = -1;
        } else
            rc = -1;
        if (rc == 0) i++;
    }
    if (rc == 0) {
        double outv = 0.0;
        if (st.size != 1 || dstack_pop(&st, &outv) != 0)
            rc = -1;
        else
            *out_y = outv;
    }
    dstack_free(&st);
    return (rc == 0) ? 0 : -1;
}
