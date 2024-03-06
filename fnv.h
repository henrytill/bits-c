#ifndef C_BITS_FNV
#define C_BITS_FNV

#include <stddef.h>
#include <stdint.h>

/// Return FNV-1a hash of input
uint64_t fnv_hash(size_t data_len, const unsigned char data[data_len]);

#endif
