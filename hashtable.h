#ifndef C_BITS_HASHTABLE_H
#define C_BITS_HASHTABLE_H

#include <stddef.h>

struct table;

struct table *table_create(size_t columns_len);

void table_destroy(struct table *t);

int table_put(struct table *t, const char *key, void *value);

void *table_get(struct table *t, const char *key);

#endif // C_BITS_HASHTABLE_H
