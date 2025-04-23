#ifndef C_BITS_HASHTABLE_H
#define C_BITS_HASHTABLE_H

#include <stddef.h>

struct table;

/// Create a new table
///
/// @param columns_len The number of columns in the table
/// @return A new table
///
/// @note The number of columns must be a power of 2
struct table *table_create(size_t columns_len);

/// Destroy a table
///
/// @param t The table to destroy
/// @param finalize A function to finalize the value of each entry
void table_destroy(struct table *t, void finalize(void *));

/// Put a key-value pair into a table
///
/// @param t The table to put the key-value pair into
/// @param key The key to put
/// @param value The value to put
/// @return 0 on success, -1 on failure
int table_put(struct table *t, char const *key, void *value);

/// Get a value from a table
///
/// @param t The table to get the value from
/// @param key The key to get
/// @return The value of the key, or NULL if the key is not found
void *table_get(struct table *t, char const *key);

/// Delete a key-value pair from a table
///
/// @param t The table to delete from
/// @param key The key to delete
/// @param finalize A function to finalize the value of the entry
/// @return 0 on success, -1 if the key is not found or on error
int table_delete(struct table *t, char const *key, void finalize(void *));

#endif // C_BITS_HASHTABLE_H
