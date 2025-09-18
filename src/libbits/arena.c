#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits.h"

/* storage allocation arena */
struct Arena {
	struct Arena *next; /* link to next arena */
	char *limit;        /* address of one past end of arena */
	char *avail;        /* next available location */
};

static struct Arena first[] = {{0}, {0}, {0}}, *arena[] = {&first[0], &first[1], &first[2]};

static size_t const minsize = (size_t)64 * 1024; /* 64KB minimum arena size */
static size_t const wordsize = sizeof(void *);
static size_t const growby = 2; /* Double arena size when growing */

static size_t
nextpage(size_t const size)
{
	static size_t pagesize = 0;
	if(pagesize == 0)
		pagesize = getpagesize();

	assert(size <= (SIZE_MAX - pagesize - 1));
	return (size + pagesize - 1) & ~(pagesize - 1);
}

static size_t
align(size_t const size, size_t const alignment)
{
	assert(ISPOW2(alignment));
	return (size + alignment - 1) & ~(alignment - 1);
}

static size_t
calcsize(size_t const n)
{
	size_t req, size;

	if(n > SIZE_MAX - sizeof(struct Arena) - minsize)
		return 0;

	req = align(n, wordsize) + sizeof(struct Arena);
	size = (req > minsize)
		       ? req * growby
		       : minsize;

	return nextpage(size);
}

void *
aalloc(size_t const n, size_t const t)
{
	struct Arena *ap;
	size_t m;

	for(ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
		if(ap->next != NULL) {
			/* move to next arena */
			ap = ap->next;
			ap->avail = (char *)ap + sizeof(*ap);
			continue;
		}

		/* allocate a new arena */
		m = calcsize(n);
		if(m == 0) {
			eprintf("allocation size too large");
			exit(EXIT_FAILURE);
		}
		ap->next = calloc(1, m);
		if(ap->next == NULL) {
			eprintf("calloc failed");
			exit(EXIT_FAILURE);
		}
		ap = ap->next;
		ap->avail = (char *)ap + sizeof(*ap);
		ap->limit = (char *)ap + m;
		ap->next = NULL;
	}
	ap->avail += n;
	return ap->avail - n;
}

void
areset(size_t const t)
{
	if((arena[t] = first[t].next) != NULL) {
		arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
		return;
	}
	arena[t] = &first[t];
}

void
afree(size_t const t)
{
	struct Arena *ap;

	ap = first[t].next;
	while(ap != NULL) {
		struct Arena *next = ap->next;
		free(ap);
		ap = next;
	}
	arena[t] = &first[t];
}
