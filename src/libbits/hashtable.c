#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "bits.h"

typedef struct entry entry;

struct entry {
	entry *next;
	char const *key;
	void *value;
};

struct table {
	size_t len;
	entry columns[1]; /* C89 flexible array member workaround */
};

table *
table_create(size_t const len)
{
	table *ret;

	if(!ISPOW2(len)) {
		debug_eprintf("len must be a power of 2\n");
		return NULL;
	}
	ret = calloc(1, sizeof(*ret) + ((len - 1) * sizeof(entry)));
	if(ret == NULL) {
		return NULL;
	}
	ret->len = len;
	return ret;
}

void
table_destroy(table *t, void finalize(void *))
{
	size_t i;
	entry *curr, *next;

	if(t == NULL) {
		return;
	}

	for(i = 0; i < t->len; ++i) {
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

static uint64_t
get_index(size_t const len, char const *key)
{
	uint64_t hash;

	assert(ISPOW2(len));
	assert(key != NULL);
	hash = fnv_hash(strlen(key) + 1, (unsigned char const *)key);
	return hash & (uint64_t)(len - 1);
}

int
table_put(table *t, char const *key, void *value)
{
	uint64_t index;
	entry *curr, *prev;

	if(t == NULL) {
		return -1;
	}
	if(key == NULL || value == NULL) {
		return -1;
	}

	index = get_index(t->len, key);
	debug_printf("key index\n");
	curr = &t->columns[index];

	while(curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		prev = curr;
		curr = curr->next;
	}
	/* existing node */
	if(curr != NULL && curr->key != NULL) {
		curr->value = value;
		return 0;
	}
	/* uninitialized key (first or deleted node) */
	if(curr != NULL) {
		curr->key = key;
		curr->value = value;
		return 0;
	}
	/* new node */
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
	uint64_t index;
	entry *curr;

	if(t == NULL) {
		return NULL;
	}
	if(key == NULL) {
		return NULL;
	}

	index = get_index(t->len, key);
	debug_printf("key index\n");
	curr = &t->columns[index];

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
	uint64_t index;
	entry *curr = NULL;
	entry *prev = NULL;

	if(t == NULL) {
		return -1;
	}
	if(key == NULL) {
		return -1;
	}

	index = get_index(t->len, key);
	curr = &t->columns[index];

	while(curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		prev = curr;
		curr = curr->next;
	}
	/* not found */
	if(curr == NULL || curr->key == NULL) {
		assert(curr->value == NULL);
		return -1;
	}
	/* found */
	if(curr->value != NULL && finalize != NULL) {
		finalize(curr->value);
	}
	if(prev == NULL) {
		/* deleting from the first entry (embedded in the table) */
		if(curr->next != NULL) {
			/* move next entry to the current entry */
			entry *next = curr->next;
			curr->key = next->key;
			curr->value = next->value;
			curr->next = next->next;
			free(next);
		} else {
			/* clear the entry */
			curr->key = NULL;
			curr->value = NULL;
		}
		return 0;
	}

	/* deleting from the chain */
	prev->next = curr->next;
	free(curr);
	return 0;
}
