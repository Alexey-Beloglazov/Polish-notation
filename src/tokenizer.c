#include <stdlib.h>
#include <string.h>

#include "token.h"

static int is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}
static int is_digit(char c) { return (c >= '0' && c <= '9'); }
static int is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

static int vec_push(TokenVec *v, Token t) {
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

void tokens_free(TokenVec *v) {
    free(v->data);
    v->data = NULL;
    v->size = v->cap = 0;
}

static int match_fn(const char *s, size_t len, FnKind *out) {
    struct {
        const char *n;
        FnKind k;
    } tbl[] = {{"sin", FN_SIN}, {"cos", FN_COS},   {"tan", FN_TAN},
               {"ctg", FN_CTG}, {"sqrt", FN_SQRT}, {"ln", FN_LN}};
    size_t i = 0;
    int rc = 1;
    while (i < sizeof(tbl) / sizeof(tbl[0])) {
        if (strlen(tbl[i].n) == len && strncmp(s, tbl[i].n, len) == 0) {
            *out = tbl[i].k;
            rc = 0;
        }
        i++;
    }
    return rc;
}

static int push_number(const char **ps, TokenVec *out_tokens, int *last) {
    int rc = 1;
    char *endp = NULL;
    double v = strtod(*ps, &endp);
    if (*ps != endp) {
        Token t = (Token){0};
        t.type = TK_NUMBER;
        t.number = v;
        rc = vec_push(out_tokens, t);
        *ps = endp;
        *last = 1;
    }
    return rc;
}

static int push_alpha(const char **ps, TokenVec *out_tokens, int *last) {
    int rc = 1;
    const char *p = *ps;
    FnKind fn;
    while (is_alpha(*p)) p++;
    if (match_fn(*ps, (size_t)(p - *ps), &fn) == 0) {
        Token t = (Token){0};
        t.type = TK_FUNCTION;
        t.fn = fn;
        rc = vec_push(out_tokens, t);
        *ps = p;
        *last = 0;
    } else
        rc = 3;
    return rc;
}

static int push_paren(const char **ps, TokenVec *out_tokens, int *last) {
    Token t = (Token){0};
    t.type = (**ps == '(') ? TK_LPAREN : TK_RPAREN;
    *last = (**ps == ')');
    (*ps)++;
    return vec_push(out_tokens, t);
}

static int push_op(const char **ps, TokenVec *out_tokens, int *last) {
    int rc = 0;
    if (**ps == '-' && !*last) {
        Token f = (Token){0};
        f.type = TK_FUNCTION;
        f.fn = FN_NEG;
        rc = vec_push(out_tokens, f);
        (*ps)++;
        *last = 0;
    } else {
        Token t = (Token){0};
        t.type = TK_OPERATOR;
        if (**ps == '+')
            t.op = OP_ADD;
        else if (**ps == '-')
            t.op = OP_SUB;
        else if (**ps == '*')
            t.op = OP_MUL;
        else if (**ps == '/')
            t.op = OP_DIV;
        else
            t.op = OP_POW;
        rc = vec_push(out_tokens, t);
        (*ps)++;
        *last = 0;
    }
    return rc;
}

int tokenize(const char *s, TokenVec *out_tokens) {
    int rc = 0, last = 0;
    out_tokens->data = NULL;
    out_tokens->size = out_tokens->cap = 0;
    while (*s && rc == 0) {
        if (is_space(*s)) {
            s++;
        } else if (is_digit(*s) || (*s == '.' && is_digit(s[1]))) {
            rc = push_number(&s, out_tokens, &last);
        } else if (*s == 'x' || *s == 'X') {
            Token t = (Token){0};
            t.type = TK_VARIABLE;
            rc = vec_push(out_tokens, t);
            s++;
            last = 1;
        } else if (is_alpha(*s)) {
            rc = push_alpha(&s, out_tokens, &last);
        } else if (*s == '(' || *s == ')') {
            rc = push_paren(&s, out_tokens, &last);
        } else if (*s == '+' || *s == '-' || *s == '*' || *s == '/' || *s == '^') {
            rc = push_op(&s, out_tokens, &last);
        } else {
            rc = 4;
        }
    }
    return rc;
}
