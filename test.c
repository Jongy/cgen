#include <stdio.h>
#include <assert.h>

#include "cgen.h"


static void simple(int n) {
    for (int i = 0; i < n; i++) {
        yield(i);
    }
}

static void sender(int n, int k) {
    for (int i = 0; i < n; i++) {
        k += yield(0);
    }
    yield(k);
}

static void many_args(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
    assert(CGEN_MAX_ARGS == 10);
    yield(a0);
    yield(a1);
    yield(a2);
    yield(a3);
    yield(a4);
    yield(a5);
    yield(a6);
    yield(a7);
    yield(a8);
    yield(a9);
}

int main(void) {
    struct gen *g;
    unsigned long val;
    int i;

    // simple test
    g = generator(simple, 10);
    val = 0;
    for (i = 0; next(g, &val); i++) {
        assert(i == val);
    }
    assert(i == 10);

    // test with send
    g = generator(sender, 5, 10);
    next(g, NULL); // prime
    val = 0;
    i = 0;
    while (send(g, &val, i++));
    assert(val == 10 + 0 + 1 + 2 + 3 + 4);

    // test with max args
    g = generator(many_args, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    val = 0;
    unsigned long sum = 0;
    while (next(g, &val)) {
        sum += val;
    }
    assert(sum == 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
}
