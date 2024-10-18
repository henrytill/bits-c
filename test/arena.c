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

static size_t get_pagesize(void)
{
    long result = sysconf(_SC_PAGESIZE);
    STATIC_ASSERT(SIZE_MAX >> 1 == LONG_MAX);
    assert(result > 0);
    return (size_t)result;
}

static inline size_t nextpage(size_t size)
{
    extern size_t pagesize;

    assert(pagesize > 0);
    assert(ISPOW2(pagesize));
    assert(size <= (SIZE_MAX - pagesize - 1));
    return (size + pagesize - 1) & ~(pagesize - 1);
}

static inline size_t calc_size(size_t n)
{
    extern const size_t MEMINCR;

    return nextpage(((n + 3) & ~3UL) + MEMINCR * 1024 + sizeof(struct arena));
}

void *arena_allocate(size_t n, size_t t)
{
    extern struct arena *arena[];

    struct arena *ap = NULL;
    for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
        if (ap->next != NULL) {
            // move to next arena
            ap = ap->next;
            ap->avail = (char *)ap + sizeof(*ap);
        } else {
            // allocate a new arena
            const size_t m = calc_size(n);
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

void arena_deallocate(size_t t)
{
    extern struct arena first[];
    extern struct arena *arena[];

    if ((arena[t] = first[t].next) != NULL) {
        arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
    } else {
        arena[t] = &first[t];
    }
}

void arena_free(size_t t)
{
    extern struct arena first[];
    extern struct arena *arena[];

    struct arena *ap = first[t].next;
    while (ap != NULL) {
        struct arena *next = ap->next;
        free(ap);
        ap = next;
    }
    arena[t] = &first[t];
}

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
