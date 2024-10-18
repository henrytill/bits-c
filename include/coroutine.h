#ifndef COROUTINE_H
#define COROUTINE_H

#include <setjmp.h>

typedef struct coroutine coroutine;

struct coroutine {
    jmp_buf callee;
    jmp_buf caller;
};

typedef void (*func)(void *);

void coroutine_start(coroutine *c, func f, void *arg, void *stack_pointer);

void coroutine_yield(coroutine *c);

int coroutine_next(coroutine *c);

#endif /* COROUTINE_H */
