#include <setjmp.h>
#include <malloc.h>
#include <stdarg.h>
#include <assert.h>

#include "cgen.h"

#define STACK_SIZE 8192

struct gen {
    union {
        unsigned char stack[STACK_SIZE];
        // this seutp struct shares space with the stack - both are never used
        // simultaneously (only stack top is used, this struct is in the bottom)
        // setup struct is used directly in asm code! make sure to update gencall.S
        // if you modify.
        struct {
            void (*func)(void);
            size_t stack_args;
            unsigned long rdi;
            unsigned long rsi;
            unsigned long rcx;
            unsigned long rdx;
            unsigned long r8;
            unsigned long r9;
        } setup;
    };

    bool started;
    bool exhausted;

    jmp_buf next;
    jmp_buf yield;
    unsigned long yield_value;
};

// see gencall.S
extern void __attribute__((noreturn)) gencall(struct gen *g, void *stack_top);

static inline unsigned long *gen_stack_top(struct gen *g) {
    return (unsigned long*)(g->stack + sizeof(g->stack));
}

static struct gen *current_gen(void) {
    register unsigned long sp asm("rsp");

    // stack is the first elemnt of the gen struct.
    return (struct gen*)(sp & ~(STACK_SIZE - 1));
}

void yield(unsigned long value) {
    struct gen *g = current_gen();

    g->yield_value = value;
    if (!setjmp(g->yield)) {
        longjmp(g->next, 1);
    }
}

bool next(struct gen *g, unsigned long *value) {
    assert(!g->exhausted); // otherwise, we're operating on dangling memory

    if (!setjmp(g->next)) {
        if (!g->started) {
            g->started = true;

            // first jump is implemented differently, since we want to pass arguments
            // and longjmp doesn't support that.
            // stack top: remove (1 + number of stack args) words. (1 for the return address)
            gencall(g, gen_stack_top(g) - 1 - g->setup.stack_args);
        } else {
            longjmp(g->yield, 1);
        }
    }

    if (g->exhausted) {
        // was marked by gen_done() because the generator returned.
        free(g);
        return false;
    }

    *value = g->yield_value;
    return true;
}

// generator functions return here when they're done.
// this jumps back to next() and notifies the generator is exhausted.
static void gen_done(void) {
    struct gen *g = current_gen();

    g->exhausted = true;
    longjmp(g->next, 1);
}

static struct gen *make_gen(void (*f)(void)) {
    struct gen *g = memalign(STACK_SIZE, sizeof(*g));

    g->started = false;
    g->exhausted = false;

    g->setup.func = f;
    g->setup.stack_args = 0;

    return g;
}

// stack_arg is one-based.
static void set_stack_arg(struct gen *g, size_t stack_arg, unsigned long arg) {
    assert(!g->started);

    if (stack_arg > g->setup.stack_args) {
        g->setup.stack_args = stack_arg;
    }

    gen_stack_top(g)[-stack_arg] = arg;
}

static bool setup_args_and_stack(struct gen *g, size_t nargs, va_list ap) {
    assert(!g->started);
    assert(nargs <= CGEN_MAX_ARGS);

    const int REG_NARGS = 6; // System V AMD64 ABI

    if (nargs > 0) g->setup.rdi = va_arg(ap, unsigned long);
    if (nargs > 1) g->setup.rsi = va_arg(ap, unsigned long);
    if (nargs > 2) g->setup.rcx = va_arg(ap, unsigned long);
    if (nargs > 3) g->setup.rdx = va_arg(ap, unsigned long);
    if (nargs > 4) g->setup.r8 = va_arg(ap, unsigned long);
    if (nargs > 5) g->setup.r9 = va_arg(ap, unsigned long);
    // stack arguments are set in reversed order
    if (nargs > REG_NARGS + 0) set_stack_arg(g, nargs - REG_NARGS - 0, va_arg(ap, unsigned long));
    if (nargs > REG_NARGS + 1) set_stack_arg(g, nargs - REG_NARGS - 1, va_arg(ap, unsigned long));
    if (nargs > REG_NARGS + 2) set_stack_arg(g, nargs - REG_NARGS - 2, va_arg(ap, unsigned long));
    if (nargs > REG_NARGS + 3) set_stack_arg(g, nargs - REG_NARGS - 3, va_arg(ap, unsigned long));

    // write the return address for the generator function
    gen_stack_top(g)[-1 - g->setup.stack_args] = (unsigned long)gen_done;
}

struct gen *gen_build(void *func, size_t nargs, ...) {
    if (nargs > CGEN_MAX_ARGS) {
        return NULL;
    }

    struct gen *g = make_gen(func);
    if (!g) {
        return NULL;
    }

    va_list ap;
    va_start(ap, nargs);
    setup_args_and_stack(g, nargs, ap);
    va_end(ap);

    return g;
}
