#ifndef C_BITS_HASHTABLE_H
#define C_BITS_HASHTABLE_H

#include <stddef.h>

/// Returns 1 if x is a power of 2
#define ISPOW2(x) (((x) & ((x)-1)) == 0)

#define STATIC_ASSERT(e) _Static_assert((e), #e)

#define TEST_ENTRIES \
    X(1, 0)          \
    X(1, 1)          \
    X(1, 2)          \
    X(0, 3)          \
    X(1, 4)          \
    X(0, 5)          \
    X(0, 6)          \
    X(0, 7)          \
    X(1, 8)          \
    X(0, 9)          \
    X(0, 15)         \
    X(1, 16)         \
    X(0, 17)         \
    X(0, 31)         \
    X(1, 32)         \
    X(0, 33)         \
    X(0, 63)         \
    X(1, 64)         \
    X(0, 65)         \
    X(1, 128)

#define X(expected, x) STATIC_ASSERT((expected) == ISPOW2((x)));
TEST_ENTRIES
#undef X

#undef TEST_ENTRIES
#undef STATIC_ASSERT

struct table;

///
/// Create a new table
///
/// @param columns_len The number of columns in the table
/// @return A new table
///
/// @note The number of columns must be a power of 2
///
struct table *table_create(size_t columns_len);

///
/// Destroy a table
///
/// @param t The table to destroy
/// @param finalize A function to finalize the value of each entry
///
void table_destroy(struct table *t, void finalize(void *));

///
/// Put a key-value pair into a table
///
/// @param t The table to put the key-value pair into
/// @param key The key to put
/// @param value The value to put
/// @return 0 on success, -1 on failure
///
int table_put(struct table *t, const char *key, void *value);

///
/// Get a value from a table
///
/// @param t The table to get the value from
/// @param key The key to get
/// @return The value of the key, or NULL if the key is not found
///
void *table_get(struct table *t, const char *key);

#endif // C_BITS_HASHTABLE_H
