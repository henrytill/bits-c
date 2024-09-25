#include "hashtable.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "fnv.h"
#include "macro.h"

#define TEST_ENTRIES \
  X(1, 0)            \
  X(1, 1)            \
  X(1, 2)            \
  X(0, 3)            \
  X(1, 4)            \
  X(0, 5)            \
  X(0, 6)            \
  X(0, 7)            \
  X(1, 8)            \
  X(0, 9)            \
  X(0, 15)           \
  X(1, 16)           \
  X(0, 17)           \
  X(0, 31)           \
  X(1, 32)           \
  X(0, 33)           \
  X(0, 63)           \
  X(1, 64)           \
  X(0, 65)           \
  X(1, 128)

#define X(expected, input) STATIC_ASSERT((expected) == ISPOW2((input)));
TEST_ENTRIES
#undef X

#undef TEST_ENTRIES
#undef STATIC_ASSERT

struct entry {
  struct entry *next;
  const char *key;
  void *value;
};

struct table {
  size_t columns_len;
  struct entry *columns;
};

struct table *table_create(const size_t columns_len) {
  if (!ISPOW2(columns_len)) {
    debug_fprintf(stderr, "%s: columns_len must be a power of 2\n", __func__);
    return NULL;
  }
  struct table *ret = calloc(1, sizeof(*ret));
  assert(ret != NULL);
  ret->columns = calloc(columns_len, sizeof(*ret->columns));
  assert(ret->columns != NULL);
  ret->columns_len = columns_len;
  return ret;
}

void table_destroy(struct table *t, void finalize(void *)) {
  if (t == NULL || t->columns == NULL) {
    return;
  }
  struct entry *curr = NULL;
  struct entry *next = NULL;
  for (size_t i = 0; i < t->columns_len; ++i) {
    for (curr = t->columns[i].next; curr != NULL;) {
      next = curr->next;
      if (finalize != NULL && curr->value != NULL) {
        finalize(curr->value);
      }
      free(curr);
      curr = next;
    }
    if (finalize != NULL && t->columns[i].value != NULL) {
      finalize(t->columns[i].value);
    }
  }
  free(t->columns);
  free(t);
}

///
/// Get the index of a key
///
/// @param columns_len The number of columns in the table
/// @param key The key to hash
/// @return The index of the key
///
/// @note The number of columns must be a power of 2
///
static uint64_t get_index(size_t columns_len, const char *key) {
  assert(ISPOW2(columns_len));
  const uint64_t hash = fnv_hash(strlen(key) + 1, (const unsigned char *)key);
  return hash & (uint64_t)(columns_len - 1);
}

int table_put(struct table *t, const char *key, void *value) {
  if (t == NULL || t->columns == NULL) {
    return -1;
  }
  if (key == NULL || value == NULL) {
    return -1;
  }

  const uint64_t index = get_index(t->columns_len, key);
  debug_printf("%s: key: %s, index: %" PRIu64 "\n", __func__, key, index);
  struct entry *curr = t->columns + index;
  struct entry *prev = NULL;

  while (curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
    prev = curr;
    curr = curr->next;
  }
  // existing node
  if (curr != NULL && curr->key != NULL) {
    curr->value = value;
    return 0;
  }
  // uninitialized key (first or deleted node)
  if (curr != NULL) {
    curr->key = key;
    curr->value = value;
    return 0;
  }
  // new node
  curr = calloc(1, sizeof(*curr));
  assert(curr != NULL);
  curr->next = NULL;
  curr->key = key;
  curr->value = value;
  assert(prev != NULL);
  prev->next = curr;
  return 0;
}

void *table_get(struct table *t, const char *key) {
  if (t == NULL || t->columns == NULL) {
    return NULL;
  }
  if (key == NULL) {
    return NULL;
  }

  const uint64_t index = get_index(t->columns_len, key);
  debug_printf("%s: key: %s, index: %" PRIu64 "\n", __func__, key, index);
  struct entry *curr = t->columns + index;

  while (curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
    curr = curr->next;
  }
  if (curr == NULL) {
    return NULL;
  }
  return curr->value;
}
