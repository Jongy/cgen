#include <stdio.h>
#include <assert.h>

#include "cgen.h"

static void mygen3(int x) {
    yield(x);
    yield(x + x);
    yield(x + x + x);
}

static void mygen2(int n, int c, int d, int e, int f, int z, int ff) {
    printf("%d %d %d %d %d %d\n", n, c, d, e, f, z);
    for (int i = 0; i < ff; i++) {
        struct gen *g = generator(mygen3, i + 1);
        unsigned long val;
        while (next(g, &val)) {
            yield(val);
        }
    }
}

static void mygen5(void) {
    yield(3);
    yield(4);
}

static void mygen6(void) {
    yield(2);
    struct gen *g = generator(mygen5);
    yield_from(g);
    yield(5);
}

static void mygen7(void) {
    yield(1);
    struct gen *g = generator(mygen6);
    yield_from(g);
    yield(6);
}

int main(void) {
    // struct gen *g = generator(mygen2, 1, 2, 3, 4, 5, 6, 9);
    struct gen *g = generator(mygen7);

    unsigned long val;
    while (next(g, &val)) {
        printf("%s: returned %ld\n", __func__, val);
    }
}
