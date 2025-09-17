#include "hashtable.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "fnv.h"
#include "macro.h"

#define TEST_ENTRIES \
	X(1, 0)      \
	X(1, 1)      \
	X(1, 2)      \
	X(0, 3)      \
	X(1, 4)      \
	X(0, 5)      \
	X(0, 6)      \
	X(0, 7)      \
	X(1, 8)      \
	X(0, 9)      \
	X(0, 15)     \
	X(1, 16)     \
	X(0, 17)     \
	X(0, 31)     \
	X(1, 32)     \
	X(0, 33)     \
	X(0, 63)     \
	X(1, 64)     \
	X(0, 65)     \
	X(1, 128)

#define X(expected, input) STATIC_ASSERT((expected) == ISPOW2((input)));
TEST_ENTRIES
#undef X

#undef TEST_ENTRIES
#undef STATIC_ASSERT

typedef struct entry entry;

struct entry {
	entry *next;
	char const *key;
	void *value;
};

typedef struct table table;

struct table {
	size_t len;
	entry columns[];
};

table *
table_create(size_t const len)
{
	if(!ISPOW2(len)) {
		debug_fprintf(stderr, "%s: len must be a power of 2\n", __func__);
		return NULL;
	}
	table *ret = calloc(1, sizeof(*ret) + (len * sizeof(entry)));
	if(ret == NULL) {
		return NULL;
	}
	ret->len = len;
	return ret;
}

void
table_destroy(table *t, void finalize(void *))
{
	if(t == NULL) {
		return;
	}
	entry *curr = NULL;
	entry *next = NULL;
	for(size_t i = 0; i < t->len; ++i) {
		curr = t->columns[i].next;
		while(curr != NULL) {
			next = curr->next;
			if(finalize != NULL && curr->value != NULL) {
				finalize(curr->value);
			}
			free(curr);
			curr = next;
		}
		if(finalize != NULL && t->columns[i].value != NULL) {
			finalize(t->columns[i].value);
		}
	}
	free(t);
}

/// Get the index of a key
///
/// @param len The number of columns in the table
/// @param key The key to hash
/// @return The index of the key
///
/// @note The number of columns must be a power of 2
static uint64_t
get_index(size_t const len, char const *key)
{
	assert(ISPOW2(len));
	assert(key != NULL);
	uint64_t const hash = fnv_hash(strlen(key) + 1, (unsigned char const *)key);
	return hash & (uint64_t)(len - 1);
}

int
table_put(table *t, char const *key, void *value)
{
	if(t == NULL) {
		return -1;
	}
	if(key == NULL || value == NULL) {
		return -1;
	}

	uint64_t const index = get_index(t->len, key);
	debug_printf("%s: key: %s, index: %" PRIu64 "\n", __func__, key, index);
	entry *curr = &t->columns[index];
	entry *prev = NULL;

	while(curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		prev = curr;
		curr = curr->next;
	}
	// existing node
	if(curr != NULL && curr->key != NULL) {
		curr->value = value;
		return 0;
	}
	// uninitialized key (first or deleted node)
	if(curr != NULL) {
		curr->key = key;
		curr->value = value;
		return 0;
	}
	// new node
	curr = calloc(1, sizeof(*curr));
	if(curr == NULL) {
		return -1;
	}
	curr->next = NULL;
	curr->key = key;
	curr->value = value;

	assert(prev != NULL);
	prev->next = curr;

	return 0;
}

void *
table_get(table *t, char const *key)
{
	if(t == NULL) {
		return NULL;
	}
	if(key == NULL) {
		return NULL;
	}

	uint64_t const index = get_index(t->len, key);
	debug_printf("%s: key: %s, index: %" PRIu64 "\n", __func__, key, index);
	entry *curr = &t->columns[index];

	while(curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		curr = curr->next;
	}
	if(curr == NULL) {
		return NULL;
	}
	return curr->value;
}

int
table_delete(table *t, char const *key, void finalize(void *))
{
	if(t == NULL) {
		return -1;
	}
	if(key == NULL) {
		return -1;
	}

	uint64_t const index = get_index(t->len, key);
	entry *curr = &t->columns[index];
	entry *prev = NULL;

	while(curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		prev = curr;
		curr = curr->next;
	}
	// not found
	if(curr == NULL || curr->key == NULL) {
		assert(curr->value == NULL);
		return -1;
	}
	// found
	if(curr->value != NULL && finalize != NULL) {
		finalize(curr->value);
	}
	if(prev == NULL) {
		// deleting from the first entry (embedded in the table)
		if(curr->next != NULL) {
			// move next entry to the current entry
			entry *next = curr->next;
			curr->key = next->key;
			curr->value = next->value;
			curr->next = next->next;
			free(next);
		} else {
			// clear the entry
			curr->key = NULL;
			curr->value = NULL;
		}
	} else {
		// deleting from the chain
		prev->next = curr->next;
		free(curr);
	}
	return 0;
}
