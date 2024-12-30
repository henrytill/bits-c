#include "fnv.h"

#include <limits.h>

#include "macro.h"

static const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;
static const uint64_t FNV_PRIME = 0x100000001b3;

// Imagine if this didn't hold...
STATIC_ASSERT(CHAR_BIT == 8);

uint64_t fnv_hash(const size_t data_len, const unsigned char data[data_len]) {
  uint64_t hash = FNV_OFFSET_BASIS;

  for (size_t i = 0; i < data_len; ++i) {
    hash ^= data[i];
    hash *= FNV_PRIME;
  }

  return hash;
}
