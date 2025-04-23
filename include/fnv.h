#ifndef C_BITS_FNV_H
#define C_BITS_FNV_H

#include <stddef.h>
#include <stdint.h>

/// Return FNV-1a hash of input
uint64_t fnv_hash(size_t data_len, unsigned char const data[data_len]);

#endif // C_BITS_FNV_H
