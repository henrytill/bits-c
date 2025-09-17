#include <limits.h>

#include "bits.h"

static uint64_t const FNV_OFFSET_BASIS = 0xcbf29ce484222325;
static uint64_t const FNV_PRIME = 0x100000001b3;

uint64_t
fnv_hash(size_t const data_len, unsigned char const *data)
{
	uint64_t hash = FNV_OFFSET_BASIS;
	size_t i;

	for(i = 0; i < data_len; ++i) {
		hash ^= data[i];
		hash *= FNV_PRIME;
	}

	return hash;
}
