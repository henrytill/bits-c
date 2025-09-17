#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "coroutine.h"

/* https://web.archive.org/web/20220528063158/https://www.embeddedrelated.com/showarticle/455.php */

enum {
	STACK_SIZE = 1024
};

struct iter {
	coroutine *c;
	int xmax;
	int ymax;
	int x;
	int y;
};

void
iterate(void *p)
{
	struct iter *it = p;

	for(int x = 0; x < it->xmax; ++x) {
		for(int y = 0; y < it->ymax; ++y) {
			it->x = x;
			it->y = y;
			coroutine_yield(it->c);
		}
	}
}

int
main(void)
{
	coroutine c;
	intptr_t stack[STACK_SIZE];
	void *stack_pointer;

	struct iter it = {
		.c = &c,
		.xmax = 2,
		.ymax = 2,
		.x = 0,
		.y = 0,
	};

	stack_pointer = stack + STACK_SIZE; /* stacks typically grow downward (arch-dependent) */
	coroutine_start(it.c, &iterate, &it, stack_pointer);
	while(coroutine_next(it.c)) {
		printf("%d %d\n", it.x, it.y);
	}
	return EXIT_SUCCESS;
}
