#include <stdio.h>
#include <cgen.h>
#include <assert.h>

static void send_gen(void) {
    unsigned long sum = 0;

    while (1) {
        unsigned long sent = yield(0);
        if (sent == 0) {
            break;
        }

        sum += sent;
    }

    yield(sum);
}

int main(void) {
    struct gen *g = generator(send_gen);
    next(g, NULL); // prime it
    for (int i = 1; i <= 10; i++) {
        send(g, NULL, i);
    }

    unsigned long sum;
    next(g, &sum);
    printf("sum is %lu\n", sum);
    assert(sum == 0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
    return 0;
}
