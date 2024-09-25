#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macro.h"

/// storage allocation arena
struct arena {
  /// link to next arena
  struct arena *next;
  /// address of one past end of arena
  char *limit;
  /// next available location
  char *avail;
};

static struct arena first[] = {{0}, {0}, {0}}, *arena[] = {&first[0], &first[1], &first[2]};

static const size_t MEMINCR = 10;

static size_t pagesize = 0;

static size_t get_pagesize(void) {
  long result = sysconf(_SC_PAGESIZE);
  STATIC_ASSERT(SIZE_MAX >> 1 == LONG_MAX);
  assert(result > 0);
  return (size_t)result;
}

static inline size_t nextpage(size_t size) {
  extern size_t pagesize;

  assert(pagesize > 0);
  assert(ISPOW2(pagesize));
  assert(size <= (SIZE_MAX - pagesize - 1));
  return (size + pagesize - 1) & ~(pagesize - 1);
}

static inline size_t calc_size(size_t n) {
  extern const size_t MEMINCR;

  return nextpage(((n + 3) & ~3UL) + MEMINCR * 1024 + sizeof(struct arena));
}

void *allocate(size_t n, size_t t) {
  extern struct arena *arena[];

  struct arena *ap = NULL;
  for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
    if (ap->next != NULL) {
      // move to next arena
      ap = ap->next;
      ap->avail = (char *)ap + sizeof(*ap);
    } else {
      // allocate a new arena
      size_t m = calc_size(n);
      ap->next = calloc(1, m);
      ap = ap->next;
      ap->limit = (char *)ap + m;
      ap->avail = (char *)ap + sizeof(*ap);
      ap->next = NULL;
    }
  }
  ap->avail += n;
  return ap->avail - n;
}

void deallocate(size_t t) {
  extern struct arena first[];
  extern struct arena *arena[];

  struct arena *ap = (arena[t] = first[t].next);
  if (ap != NULL) {
    arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
  } else {
    arena[t] = &first[t];
  }
}

void unmap(size_t t) {
  extern struct arena first[];
  extern struct arena *arena[];

  struct arena *ap = first[t].next;
  struct arena *prev = &first[t];

  while (ap != NULL) {
    struct arena *next = ap->next;
    free(ap);
    prev->next = next;
    ap = next;
  }
  arena[t] = &first[t];
}

int main(void) {
  extern size_t pagesize;

  pagesize = get_pagesize();

  struct point {
    int x;
    int y;
  };

  struct point *p = allocate(sizeof(*p), 0);
  p->x = 4;
  p->y = 2;
  printf("p = {x = %d, y = %d}\n", p->x, p->y);

  struct point *q = allocate(sizeof(*q), 0);
  q->x = 5;
  q->y = 3;
  printf("q = {x = %d, y = %d}\n", q->x, q->y);

  deallocate(0);

  unmap(0);

  return EXIT_SUCCESS;
}
