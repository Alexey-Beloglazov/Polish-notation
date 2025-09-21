#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "token.h"

static void draw(char *buf, int w, int h) {
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            putchar(buf[r * w + c]);
        }
        if (r + 1 < h) putchar('\n');
    }
}

int main(void) {
    char line[4096];
    if (!fgets(line, sizeof(line), stdin)) {
        puts("n/a");
        return 1;
    }
    line[strcspn(line, "\n")] = '\0';
    if (!*line) {
        puts("n/a");
        return 1;
    }

    TokenVec toks, rpn;
    if (tokenize(line, &toks) != 0) {
        puts("n/a");
        return 1;
    }
    if (to_rpn(toks.data, toks.size, &rpn) != 0) {
        tokens_free(&toks);
        puts("n/a");
        return 1;
    }

    char *canvas = (char *)malloc(WIDTH * HEIGHT);
    if (!canvas) {
        tokens_free(&toks);
        tokens_free(&rpn);
        puts("n/a");
        return 1;
    }
    memset(canvas, '.', WIDTH * HEIGHT);

    double dx = (RIGHT_BORDER - LEFT_BORDER) / (WIDTH - 1);
    int any_ok = 0;

    for (int i = 0; i < WIDTH; i++) {
        double x = LEFT_BORDER + i * dx;
        double y = 0.0;

        if (eval_rpn(rpn.data, rpn.size, x, &y) != 0) {
            continue;
        }
        if (isnan(y) || isinf(y) || y < BOTTOM_BORDER || y > TOP_BORDER) {
            continue;
        }

        int row = (int)llround((TOP_BORDER - y) * (HEIGHT - 1) / (TOP_BORDER - BOTTOM_BORDER));
        if (row < 0) row = 0;
        if (row >= HEIGHT) row = HEIGHT - 1;

        canvas[row * WIDTH + i] = '*';
        any_ok = 1;
    }

    if (!any_ok)
        puts("n/a");
    else
        draw(canvas, WIDTH, HEIGHT);

    free(canvas);
    tokens_free(&toks);
    tokens_free(&rpn);
    return 0;
}
