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
	struct arena *next; ///< link to next arena
	char *limit;        ///< address of one past end of arena
	char *avail;        ///< next available location
};

static struct arena first[] = {{0}, {0}, {0}}, *arena[] = {&first[0], &first[1], &first[2]};

static size_t const MIN_ARENA_SIZE = (size_t)64 * 1024; // 64KB minimum arena size
static size_t const WORD_SIZE = sizeof(void *);
static size_t const GROWTH_FACTOR = 2; // Double arena size when growing

static size_t get_pagesize(void)
{
	long const result = sysconf(_SC_PAGESIZE);
	STATIC_ASSERT(SIZE_MAX >> 1 == LONG_MAX);
	assert(result > 0);
	assert(ISPOW2(result));
	return (size_t)result;
}

static inline size_t nextpage(size_t const size)
{
	static size_t pagesize = 0;
	if (pagesize == 0) {
		pagesize = get_pagesize();
	}

	assert(size <= (SIZE_MAX - pagesize - 1));
	return (size + pagesize - 1) & ~(pagesize - 1);
}

static inline size_t align_up(size_t const size, size_t const alignment)
{
	assert(ISPOW2(alignment));
	return (size + alignment - 1) & ~(alignment - 1);
}

static inline size_t calc_size(size_t const n)
{
	if (n > SIZE_MAX - sizeof(struct arena) - MIN_ARENA_SIZE) {
		return 0;
	}

	size_t const aligned_n = align_up(n, WORD_SIZE);

	size_t const requested_total = aligned_n + sizeof(struct arena);

	size_t const arena_size = (requested_total > MIN_ARENA_SIZE)
					  ? requested_total * GROWTH_FACTOR
					  : MIN_ARENA_SIZE;

	return nextpage(arena_size);
}

void *arena_allocate(size_t const n, size_t const t)
{
	struct arena *ap = NULL;
	for (ap = arena[t]; ap->avail + n > ap->limit; arena[t] = ap) {
		if (ap->next != NULL) {
			// move to next arena
			ap = ap->next;
			ap->avail = (char *)ap + sizeof(*ap);
			continue;
		}

		// allocate a new arena
		size_t const m = calc_size(n);
		if (m == 0) {
			eprintf("%s: allocation size too large", __func__);
			exit(EXIT_FAILURE);
		}
		ap->next = calloc(1, m);
		if (ap->next == NULL) {
			eprintf("%s: calloc failed", __func__);
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

void arena_deallocate(size_t const t)
{
	if ((arena[t] = first[t].next) != NULL) {
		arena[t]->avail = (char *)arena[t] + sizeof(*arena[t]);
		return;
	}
	arena[t] = &first[t];
}

void arena_free(size_t const t)
{
	struct arena *ap = first[t].next;
	while (ap != NULL) {
		struct arena *next = ap->next;
		free(ap);
		ap = next;
	}
	arena[t] = &first[t];
}
