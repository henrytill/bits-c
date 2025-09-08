#include "fnv.h"

#include <limits.h>

#include "macro.h"

static uint64_t const FNV_OFFSET_BASIS = 0xcbf29ce484222325;
static uint64_t const FNV_PRIME = 0x100000001b3;

// Imagine if this didn't hold...
STATIC_ASSERT(CHAR_BIT == 8);

uint64_t fnv_hash(size_t const data_len, unsigned char const data[data_len])
{
	uint64_t hash = FNV_OFFSET_BASIS;

	for (size_t i = 0; i < data_len; ++i) {
		hash ^= data[i];
		hash *= FNV_PRIME;
	}

	return hash;
}
