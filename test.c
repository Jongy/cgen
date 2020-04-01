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

static void test_simple(void) {
    struct gen *g = generator(simple, 10);
    unsigned long val, i;
    for (i = 0; next(g, &val); i++) {
        assert(i == val);
    }
    assert(i == 10);
}

static void sender(int n, int k) {
    for (int i = 0; i < n; i++) {
        k += yield(0);
    }
    yield(k);
}

static void test_send(void) {
    struct gen *g = generator(sender, 5, 10);
    next(g, NULL); // prime
    unsigned long val, i = 0;
    while (send(g, &val, i++));
    assert(val == 10 + 0 + 1 + 2 + 3 + 4);
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

static void test_max_args(void) {
    struct gen *g = generator(many_args, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    unsigned long sum = 0, val;
    while (next(g, &val)) {
        sum += val;
    }
    assert(sum == 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
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

static void _test_yieldfrom(struct gen *g, int add) {
    unsigned long val, sum = 0;
    assert(next(g, &val) && val == 42);
    while (next(g, &val)) {
        sum += val;
    }
    assert(sum == 0 + 1 + 2 + 3 + 4 + 0 + 1 + 2 + 3 + 4 + 5 + 42 + add);
}

static void test_yieldfrom(void) {
    struct gen *g = generator(yieldfrom, 5);
    _test_yieldfrom(g, 0);
}

static void yieldfrom2(int n) {
    struct gen *g = generator(yieldfrom, n);
    yield_from(g);
    yield(17);
}

static void yieldfrom3(int n) {
    struct gen *g = generator(yieldfrom2, n);
    yield_from(g);
    yield(12);
}

static void test_yieldfrom_chained(void) {
    struct gen *g = generator(yieldfrom3, 5);
    _test_yieldfrom(g, 17 + 12);
}

int main(void) {
    test_simple();
    test_send();
    test_max_args();
    test_yieldfrom();
    test_yieldfrom_chained();
}
