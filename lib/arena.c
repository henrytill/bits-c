#include "arena.h"

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

static size_t const MEMINCR = 10;

static size_t pagesize = 0;

size_t get_pagesize(void) {
  long const result = sysconf(_SC_PAGESIZE);
  STATIC_ASSERT(SIZE_MAX >> 1 == LONG_MAX);
  assert(result > 0);
  return (size_t)result;
}

void arena_init(void) {
  pagesize = get_pagesize();
}

static inline size_t nextpage(size_t const size) {
  assert(pagesize > 0);
  assert(ISPOW2(pagesize));
  assert(size <= (SIZE_MAX - pagesize - 1));
  return (size + pagesize - 1) & ~(pagesize - 1);
}

static inline size_t calc_size(size_t const n) {
  return nextpage(((n + 3) & ~3UL) + (MEMINCR * 1024) + sizeof(struct arena));
}

void *arena_allocate(size_t const n, size_t const t) {
  struct arena *ap = NULL;
  for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
    if (ap->next != NULL) {
      // move to next arena
      ap = ap->next;
      ap->avail = (char *)ap + sizeof(*ap);
    } else {
      // allocate a new arena
      size_t const m = calc_size(n);
      ap->next = calloc(1, m);
      if (ap->next == NULL) {
        eprintf("%s: calloc failed", __func__);
        exit(EXIT_FAILURE);
      }
      ap = ap->next;
      ap->limit = (char *)ap + m;
      ap->avail = (char *)ap + sizeof(*ap);
      ap->next = NULL;
    }
  }
  ap->avail += n;
  return ap->avail - n;
}

void arena_deallocate(size_t const t) {
  if ((arena[t] = first[t].next) != NULL) {
    arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
  } else {
    arena[t] = &first[t];
  }
}

void arena_free(size_t const t) {
  struct arena *ap = first[t].next;
  while (ap != NULL) {
    struct arena *next = ap->next;
    free(ap);
    ap = next;
  }
  arena[t] = &first[t];
}
