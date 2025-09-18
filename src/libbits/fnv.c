#include <limits.h>

#include "bits.h"

static uint64_t const offsetbasis = 0xcbf29ce484222325;
static uint64_t const prime = 0x100000001b3;

uint64_t
fnv(size_t const datalen, unsigned char const *data)
{
	uint64_t hash = offsetbasis;
	size_t i;

	for(i = 0; i < datalen; ++i) {
		hash ^= data[i];
		hash *= prime;
	}

	return hash;
}
