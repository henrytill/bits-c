#include <stdio.h>
#include <stdlib.h>

#include "arena.h"

int main(void) {
  arena_init();

  struct point {
    int x;
    int y;
  };

#define PRINT_POINT(_pt) printf(#_pt " = {x = %d, y = %d}\n", (_pt)->x, (_pt)->y)

  struct point *p = arena_allocate(sizeof(*p), 0);
  p->x = 0;
  p->y = 1;
  PRINT_POINT(p);

  struct point *q = arena_allocate(sizeof(*q), 0);
  q->x = 2;
  q->y = 3;
  PRINT_POINT(q);

  arena_deallocate(0);

  struct point *r = arena_allocate(sizeof(*r), 0);
  r->x = 4;
  r->y = 5;
  PRINT_POINT(r);

  struct point *s = arena_allocate(sizeof(*s), 0);
  s->x = 6;
  s->y = 7;
  PRINT_POINT(s);

#undef PRINT_POINT

  arena_free(0);

  return EXIT_SUCCESS;
}
