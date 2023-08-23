#ifndef C_BITS_ALLOC
#define C_BITS_ALLOC

#include <stddef.h>

///
/// Allocate or die.
///
/// @param size The size in bytes to allocate.
/// @return A pointer to the allocated memory.
///
void *emalloc(size_t size);

///
/// Allocate and zero or die.
///
/// @param nmemb The number of elements to allocate.
/// @param size The size in bytes of each element.
/// @return A pointer to the allocated memory.
///
void *ecalloc(size_t nmemb, size_t size);

#endif
