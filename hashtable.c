#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "fnv.h"
#include "macro.h"

/// Returns 1 if x is a power of 2
#define ISPOW2(x) (((x) & ((x)-1)) == 0)

struct entry {
  struct entry *next;
  const char *key;
  void *value;
};

struct table {
  size_t columns_len;
  struct entry *columns;
};

static struct table *table_create(const size_t columns_len) {
  if (!ISPOW2(columns_len)) {
    (void)fprintf(stderr, "%s: columns_len must be a power of 2\n", __func__);
    return NULL;
  }
  struct table *ret = ecalloc(1, sizeof(*ret));
  ret->columns = ecalloc(columns_len, sizeof(*ret->columns));
  ret->columns_len = columns_len;
  return ret;
}

static void table_destroy(struct table *t) {
  if (t == NULL || t->columns == NULL) {
    return;
  }
  struct entry *curr = NULL;
  struct entry *next = NULL;
  for (size_t i = 0; i < t->columns_len; ++i) {
    for (curr = t->columns[i].next; curr != NULL;) {
      next = curr->next;
      free(curr);
      curr = next;
    }
  }
  free(t->columns);
  free(t);
}

static uint64_t get_index(size_t columns_len, const char *key) {
  assert(ISPOW2(columns_len));
  const uint64_t hash = fnv_hash(strlen(key) + 1, (const unsigned char *)key);
  return hash & (uint64_t)(columns_len - 1);
}

static int table_put(struct table *t, const char *key, void *value) {
  if (t == NULL || t->columns == NULL) {
    return -1;
  }
  if (key == NULL || value == NULL) {
    return -1;
  }

  const uint64_t index = get_index(t->columns_len, key);
  (void)debug_printf("%s: key: %s, index: %" PRIu64 "\n", __func__, key, index);
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
  curr = ecalloc(1, sizeof(*curr));
  curr->next = NULL;
  curr->key = key;
  curr->value = value;
  assert(prev != NULL);
  prev->next = curr;
  return 0;
}

static void *table_get(struct table *t, const char *key) {
  if (t == NULL || t->columns == NULL) {
    return NULL;
  }

  const uint64_t index = get_index(t->columns_len, key);
  struct entry *curr = t->columns + index;

  while (curr != NULL && strcmp(key, curr->key) != 0) {
    curr = curr->next;
  }
  if (curr == NULL) {
    return NULL;
  }
  return curr->value;
}

static const struct test_vector {
  const char *key;
  char *value;
} TEST_VECTORS[] = {
#define X(prefix) {#prefix "_key", #prefix "_value"},
#include "hashtable_vectors.def"
#undef X
  {NULL, NULL},
};

int main(void) {
  extern const struct test_vector TEST_VECTORS[];

  int ret = EXIT_FAILURE;
  const char *key = NULL;

  if (fnv_hash_test() == false) {
    return EXIT_FAILURE;
  }

  struct table *t = table_create(8);

  for (size_t i = 0; (key = TEST_VECTORS[i].key) != NULL; ++i) {
    (void)table_put(t, key, TEST_VECTORS[i].value);
  }

  for (size_t i = 0; (key = TEST_VECTORS[i].key) != NULL; ++i) {
    char *value = table_get(t, key);
    if (strcmp(TEST_VECTORS[i].value, value) != 0) {
      goto out_table_destroy;
    }
  }

  ret = EXIT_SUCCESS;
out_table_destroy:
  table_destroy(t);
  return ret;
}
