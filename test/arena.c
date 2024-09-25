#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
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
  static_assert(SIZE_MAX >> 1 == LONG_MAX, "SIZE_MAX >> 1 != LONG_MAX");
  assert(result > 0);
  return (size_t)result;
}

static size_t nextpage(size_t size) {
  extern size_t pagesize;

  assert(pagesize > 0);
  assert(ISPOW2(pagesize));
  assert(size <= (SIZE_MAX - pagesize - 1));
  return (size + pagesize - 1) & ~(pagesize - 1);
}

static void *morecore(size_t size) {
  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }
  debug_printf("mapped %zu bytes at address range %p-%p\n", size, ptr, (char *)ptr + size);
  return ptr;
}

void *allocate(size_t n, size_t t) {
  extern struct arena *arena[];
  extern const size_t MEMINCR;

  struct arena *ap = NULL;
  for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
    if (ap->next != NULL) {
      // move to next arena
      ap = ap->next;
      ap->avail = (char *)ap + sizeof(*ap);
    } else {
      // allocate a new arena
      size_t m = nextpage(((n + 3) & ~3UL) + MEMINCR * 1024 + sizeof(*ap));
      ap->next = morecore(m);
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
    size_t size = (size_t)(ap->limit - (char *)ap);
    debug_printf("unmapped %zu bytes at address %p\n", size, ap);
    if (munmap(ap, size) == 0) {
      prev->next = next;
    } else {
      prev = ap;
    }
    ap = next;
  }
  arena[t] = &first[t];
}

#ifdef DEBUG
static void print_maps(void) {
  FILE *fp = fopen("/proc/self/maps", "r");
  if (fp != NULL) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      printf("%s", line);
    }
    (void)fclose(fp);
  }
}
#else
static void print_maps(void) {}
#endif

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

  print_maps();

  debug_printf("=== deallocating ===\n");
  deallocate(0);
  print_maps();

  debug_printf("=== unmapping ===\n");
  unmap(0);
  print_maps();

  return EXIT_SUCCESS;
}
