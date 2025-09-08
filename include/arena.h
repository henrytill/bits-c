#ifndef C_BITS_ARENA_H
#define C_BITS_ARENA_H

#include <stddef.h>

/// Initialize the arena allocator system
///
/// Must be called before any other arena functions.
/// Sets up the page size for memory alignment.
void arena_init(void);

/// Allocate memory from an arena
///
/// @param n The number of bytes to allocate
/// @param t The arena type/index (0, 1, or 2)
/// @return Pointer to allocated memory, or exits on failure
///
/// @note Memory is word-aligned for optimal performance
/// @note New arenas are created automatically as needed
/// @note Does not initialize the allocated memory
void *arena_allocate(size_t n, size_t t);

/// Reset an arena to its first allocated block
///
/// @param t The arena type/index (0, 1, or 2)
///
/// @note Does not free memory, just resets the allocation pointer
/// @note Subsequent allocations will reuse existing arena blocks
void arena_deallocate(size_t t);

/// Free all memory blocks in an arena
///
/// @param t The arena type/index (0, 1, or 2)
///
/// @note Completely deallocates all arena blocks for the given type
/// @note Arena can still be used after calling this function
void arena_free(size_t t);

#endif // C_BITS_ARENA_H
