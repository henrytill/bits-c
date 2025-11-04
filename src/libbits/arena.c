#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits.h"
#include "macro.h"
#include "printf.h"

/* storage allocation arena */
struct Arena
{
    struct Arena *next; /**< link to next arena */
    char *limit;        /**< address of one past end of arena */
    char *avail;        /**< next available location */
};

static int const arenasize = sizeof(struct Arena);
static int const maxalign = sizeof(long double);
static int const minsize = 64 * 1024;

static struct Arena first[] = { { 0 }, { 0 }, { 0 } }, *arena[] = { &first[0], &first[1], &first[2] };

static int const narena = NELEM(arena);

static void init(void)
{
    static int inited = 0;
    int i;

    if (inited)
        return;

    for (i = 0; i < narena; ++i)
    {
        first[i].avail = first[i].limit = (char *)&first[i];
        first[i].next = NULL;
    }

    inited = 1;
}

static int nextpage(int const size)
{
    static int pagesize = 0;
    long ps;

    if (pagesize == 0)
    {
        ps = sysconf(_SC_PAGESIZE);
        if (ps <= 0)
        {
            perror("failed to get page size");
            exit(EXIT_FAILURE);
        }
        pagesize = (assert(ps <= INT_MAX), (int)ps);
    }

    if (size > INT_MAX - pagesize - 1)
        return -1;

    if (ISPOW2(pagesize))
        return (size + pagesize - 1) & ~(pagesize - 1);

    return ((size + pagesize - 1) / pagesize) * pagesize;
}

static int align(int const size, int const alignment)
{
    assert(ISPOW2(alignment));

    if (size > INT_MAX - alignment - 1)
        return -1;

    return (size + alignment - 1) & ~(alignment - 1);
}

static int calcsize(int n)
{
    if (n > INT_MAX - arenasize)
        return -1;

    n += arenasize;
    if (n > minsize)
    {
        /* Check if left shift by 1 would overflow */
        if (n & (INT_MAX ^ (INT_MAX >> 1)))
            return -1;
        n <<= 1;
    }
    else
    {
        n = minsize;
    }

    return nextpage(n);
}

void *aalloc(int n, int const t)
{
    struct Arena *ap;
    int s;

    if (n <= 0)
        return NULL;

    if (t < 0 || t >= narena)
        return NULL;

    init();

    n = align(n, maxalign);
    if (n == -1)
    {
        eprintf("allocation size too large");
        return NULL;
    }

    for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap)
    {
        if (ap->next != NULL)
        {
            /* move to next arena */
            ap = ap->next;
            ap->avail = (char *)ap + sizeof(*ap);
            continue;
        }

        /* allocate a new arena */
        s = calcsize(n);
        if (s == -1)
        {
            eprintf("allocation size too large");
            exit(EXIT_FAILURE);
        }
        ap->next = calloc(1, s);
        if (ap->next == NULL)
        {
            eprintf("calloc failed");
            exit(EXIT_FAILURE);
        }
        ap = ap->next;
        ap->avail = (char *)ap + sizeof(*ap);
        ap->limit = (char *)ap + s;
        ap->next = NULL;
    }

    ap->avail += n;
    return ap->avail - n;
}

void areset(int const t)
{
    if (t < 0 || t >= narena)
    {
        eprintf("unknown arena: %d\n", t);
        return;
    }

    if ((arena[t] = first[t].next) != NULL)
    {
        arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
        return;
    }

    arena[t] = &first[t];
}

void afree(int const t)
{
    struct Arena *ap, *next;

    if (t < 0 || t >= narena)
    {
        eprintf("unknown arena: %d\n", t);
        return;
    }

    ap = first[t].next;
    while (ap != NULL)
    {
        next = ap->next;
        free(ap);
        ap = next;
    }

    arena[t] = &first[t];
}
