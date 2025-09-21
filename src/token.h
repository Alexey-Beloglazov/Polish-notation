#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

typedef enum { TK_NUMBER, TK_VARIABLE, TK_OPERATOR, TK_FUNCTION, TK_LPAREN, TK_RPAREN } TokenType;

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_POW } OpKind;
typedef enum { FN_SIN, FN_COS, FN_TAN, FN_CTG, FN_SQRT, FN_LN, FN_NEG } FnKind;

typedef struct {
    TokenType type;
    double number;
    OpKind op;
    FnKind fn;
} Token;

typedef struct {
    Token *data;
    size_t size;
    size_t cap;
} TokenVec;

int tokenize(const char *s, TokenVec *out_tokens);
void tokens_free(TokenVec *v);
int to_rpn(const Token *in, size_t n, TokenVec *out_rpn);
int eval_rpn(const Token *rpn, size_t n, double x_value, double *out_y);

#endif
