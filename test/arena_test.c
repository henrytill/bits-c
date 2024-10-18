#include <stdio.h>
#include <stdlib.h>

#include "arena.h"

int main(void)
{
    extern size_t pagesize;

    pagesize = get_pagesize();

    struct point {
        int x;
        int y;
    };

#define PRINT_POINT(_pt) printf(#_pt " = {x = %d, y = %d}\n", (_pt)->x, (_pt)->y)

    struct point *p = arena_allocate(sizeof(*p), 0);
    p->x = 4;
    p->y = 2;
    PRINT_POINT(p);

    struct point *q = arena_allocate(sizeof(*q), 0);
    q->x = 5;
    q->y = 3;
    PRINT_POINT(q);

    arena_deallocate(0);

    struct point *r = arena_allocate(sizeof(*r), 0);
    r->x = 4;
    r->y = 2;
    PRINT_POINT(r);

    struct point *s = arena_allocate(sizeof(*s), 0);
    s->x = 5;
    s->y = 3;
    PRINT_POINT(s);

#undef PRINT_POINT

    arena_free(0);

    return EXIT_SUCCESS;
}
