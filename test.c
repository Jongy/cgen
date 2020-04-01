#include <stdio.h>
#include <assert.h>

#include "cgen.h"

#ifdef NDEBUG
#error compile me with asserts, thanks
#endif


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

static void yieldfrom(int n) {
    struct gen *g;

    yield(42);

    g = generator(simple, n);
    yield_from(g);
    g = generator(simple, n + 1);
    yield_from(g);

    yield(42);
}

int main(void) {
    struct gen *g;
    unsigned long val;
    int i;

    // simple test
    g = generator(simple, 10);
    for (i = 0; next(g, &val); i++) {
        assert(i == val);
    }
    assert(i == 10);

    // test with send
    g = generator(sender, 5, 10);
    next(g, NULL); // prime
    i = 0;
    while (send(g, &val, i++));
    assert(val == 10 + 0 + 1 + 2 + 3 + 4);

    // test with max args
    g = generator(many_args, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    unsigned long sum = 0;
    while (next(g, &val)) {
        sum += val;
    }
    assert(sum == 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);

    // test yield from
    g = generator(yieldfrom, 5);
    assert(next(g, &val) && val == 42);
    sum = 0;
    while (next(g, &val)) {
        if (val == 42) {
            break;
        }

        sum += val;
    }
    assert(val == 42);
    assert(sum == 0 + 1 + 2 + 3 + 4 + 0 + 1 + 2 + 3 + 4 + 5);
}
