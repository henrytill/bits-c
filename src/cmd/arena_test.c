#include <stdio.h>
#include <stdlib.h>

#include "bits.h"

int main(void) {
  struct Point {
    int x;
    int y;
  };

#define PRINT_POINT(_pt) printf(#_pt " = {x = %d, y = %d}\n", (_pt)->x, (_pt)->y)

  {
    struct Point *p = aalloc(sizeof(*p), 0);

    p->x = 0;
    p->y = 1;
    PRINT_POINT(p);
  }

  {
    struct Point *q = aalloc(sizeof(*q), 0);

    q->x = 2;
    q->y = 3;
    PRINT_POINT(q);
  }

  areset(0);

  {
    struct Point *r = aalloc(sizeof(*r), 0);

    r->x = 4;
    r->y = 5;
    PRINT_POINT(r);
  }

  {
    struct Point *s = aalloc(sizeof(*s), 0);

    s->x = 6;
    s->y = 7;
    PRINT_POINT(s);
  }

#undef PRINT_POINT

  afree(0);

  return EXIT_SUCCESS;
}
