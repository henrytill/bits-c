#ifndef C_BITS_ARENA_H
#define C_BITS_ARENA_H

#include <stddef.h>

void arena_init(void);

void *arena_allocate(size_t n, size_t t);

void arena_deallocate(size_t t);

void arena_free(size_t t);

#endif
