#include "fnv.h"

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;
static const uint64_t FNV_PRIME = 0x100000001b3;

// Imagine if this didn't hold...
_Static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

/// Return FNV-1a hash of input
uint64_t fnv_hash(const size_t data_len, const unsigned char data[data_len])
{
    extern const uint64_t FNV_OFFSET_BASIS;
    extern const uint64_t FNV_PRIME;

    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < data_len; ++i) {
        hash ^= data[i];
        hash *= FNV_PRIME;
    }

    return hash;
}
