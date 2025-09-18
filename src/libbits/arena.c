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

static int const arenasize = sizeof(struct Arena);
static int const wordsize = sizeof(void *);
static int const minsize = 64 * 1024;
static int const factor = 2;

static struct Arena first[] = {{0}, {0}, {0}}, *arena[] = {&first[0], &first[1], &first[2]};

static int const narena = NELEM(arena);

static int
nextpage(int const size)
{
	static int pagesize = 0;
	if(pagesize == 0)
		pagesize = getpagesize();

	assert(size <= (INT_MAX - pagesize - 1));
	return (size + pagesize - 1) & ~(pagesize - 1);
}

static int
align(int const size, int const alignment)
{
	assert(ISPOW2(alignment));
	return (size + alignment - 1) & ~(alignment - 1);
}

static int
calcsize(int const n)
{
	int req, size;

	if(n > INT_MAX - arenasize - minsize)
		return -1;

	req = align(n, wordsize) + arenasize;
	size = (req > minsize) ? req * factor : minsize;

	return nextpage(size);
}

void *
aalloc(int const n, int const t)
{
	struct Arena *ap;
	int s;

	assert(t >= 0 && t < narena);

	for(ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
		if(ap->next != NULL) {
			/* move to next arena */
			ap = ap->next;
			ap->avail = (char *)ap + sizeof(*ap);
			continue;
		}

		/* allocate a new arena */
		s = calcsize(n);
		if(s == -1) {
			eprintf("allocation size too large");
			exit(EXIT_FAILURE);
		}
		ap->next = calloc(1, s);
		if(ap->next == NULL) {
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

void
areset(int const t)
{
	assert(t >= 0 && t < narena);

	if((arena[t] = first[t].next) != NULL) {
		arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
		return;
	}
	arena[t] = &first[t];
}

void
afree(int const t)
{
	struct Arena *ap;

	assert(t >= 0 && t < narena);

	ap = first[t].next;
	while(ap != NULL) {
		struct Arena *next = ap->next;
		free(ap);
		ap = next;
	}
	arena[t] = &first[t];
}
