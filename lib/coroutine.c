#include "coroutine.h"

#define get_stack_pointer(p) __asm__ volatile("movq %%rsp, %0" \
	: "=r"(p))

#define get_frame_pointer(p) __asm__ volatile("movq %%rbp, %0" \
	: "=r"(p))

#define set_stack_pointer(p) __asm__ volatile("movq %0, %%rsp" \
	: \
	: "r"(p))

#define set_frame_pointer(p) __asm__ volatile("movq %0, %%rbp" \
	: \
	: "r"(p))

enum {
	WORKING = 1,
	DONE
};

enum {
	FRAME_SIZE = 5 /* fairly arbitrary */
};

struct start_params {
	coroutine *c;
	func f;
	void *arg;
	void *old_stack_pointer;
	void *old_frame_pointer;
};

void coroutine_yield(coroutine *c)
{
	if (setjmp(c->callee) == 0) {
		longjmp(c->caller, WORKING);
	}
}

int coroutine_next(coroutine *c)
{
	int ret = setjmp(c->caller);
	if (ret == 0) {
		longjmp(c->callee, WORKING);
	} else {
		return ret == WORKING;
	}
}

void coroutine_start(coroutine *c, func f, void *arg, void *stack_pointer)
{
	struct start_params *p = ((struct start_params *)stack_pointer) - 1;

	/* save params before stack switching */
	p->c = c;
	p->f = f;
	p->arg = arg;
	get_stack_pointer(p->old_stack_pointer);
	get_frame_pointer(p->old_frame_pointer);
	set_stack_pointer(p - FRAME_SIZE);
	set_frame_pointer(p); /* effectively clobbers and all other locals... */
	/* ...so we read p back from $fp */
	get_frame_pointer(p);
	/* and now we read our params from p */
	if (setjmp(p->c->callee) == 0) {
		set_stack_pointer(p->old_stack_pointer);
		set_frame_pointer(p->old_frame_pointer);
		return;
	}
	(*p->f)(p->arg);
	longjmp(p->c->caller, DONE);
}
