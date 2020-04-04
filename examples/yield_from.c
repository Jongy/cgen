#include <stdio.h>
#include <assert.h>
#include <cgen.h>

void simple_gen(int up_to) {
    for (int i = 1; i <= up_to; i++) {
        yield(i);
    }
}

void yield_from_gen(int n) {
    for (int i = 0; i < n; i++) {
        struct gen *g = generator(simple_gen, 10);
        yield_from(g);
    }
}

int main(void) {
    struct gen *g = generator(yield_from_gen, 3);
    unsigned long val;
    unsigned long sum = 0;
    while (next(g, &val)) {
        sum += val;
    }
    printf("sum is %lu\n", sum);
    assert(sum == 3 * (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10));
    return 0;
}
