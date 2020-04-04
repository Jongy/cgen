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

static void _test_yieldfrom_chained_bidirectional3(void) {
    assert(30 == yield(3));
    assert(40 == yield(4));
}

static void _test_yieldfrom_chained_bidirectional2(void) {
    assert(20 == yield(2));
    struct gen *g = generator(_test_yieldfrom_chained_bidirectional3);
    yield_from(g);
    assert(50 == yield(5));
}

static void _test_yieldfrom_chained_bidirectional1(void) {
    assert(0 == yield(0)); // for the first next
    assert(10 == yield(1));
    struct gen *g = generator(_test_yieldfrom_chained_bidirectional2);
    yield_from(g);
    assert(60 == yield(6));
    yield(0);
}

/*
    I was getting really confused by this point, so I wrote a test in Python to get
    another source of verification...

    Copy it to an empty file and run with pytest:

def test():
    def t3():
        assert 30 == (yield 3)
        assert 40 == (yield 4)

    def t2():
        assert 20 == (yield 2)
        yield from t3()
        assert 50 == (yield 5)

    def t1():
        assert 0 == (yield 0)
        assert 10 == (yield 1)
        yield from t2()
        assert 60 == (yield 6)
        yield 0

    g = t1()
    assert (g.send(None) == 0 and
            g.send(0) == 1 and
            g.send(10) == 2 and
            g.send(20) == 3 and
            g.send(30) == 4 and
            g.send(40) == 5 and
            g.send(50) == 6 and
            g.send(60) == 0)

    import pytest
    with pytest.raises(StopIteration):
        next(g)

*/

static void test_yieldfrom_chained_bidirectional(void) {
    struct gen *g = generator(_test_yieldfrom_chained_bidirectional1);
    unsigned long val;
    assert(next(g, &val) && val == 0);
    assert(send(g, &val, 0) && val == 1);
    assert(send(g, &val, 10) && val == 2);
    assert(send(g, &val, 20) && val == 3);
    assert(send(g, &val, 30) && val == 4);
    assert(send(g, &val, 40) && val == 5);
    assert(send(g, &val, 50) && val == 6);
    assert(send(g, &val, 60) && val == 0);
}

int main(void) {
    test_simple();
    test_send();
    test_max_args();
    test_yieldfrom();
    test_yieldfrom_chained();
    test_yieldfrom_chained_bidirectional();
}
