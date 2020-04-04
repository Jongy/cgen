#include <stdio.h>
#include <assert.h>
#include <cgen.h>

static void simple_gen(int up_to) {
    for (int i = 1; i <= up_to; i++) {
        yield(i);
    }
}

int main(void) {
    struct gen *g = generator(simple_gen, 10);
    unsigned long val;
    unsigned long sum = 0;
    while (next(g, &val)) {
        printf("got %lu\n", val);
        sum += val;
    }
    assert(sum == 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
    return 0;
}
