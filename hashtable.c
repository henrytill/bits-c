#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.h"

struct entry {
	struct entry *next;
	const char *key;
	void *value;
};

struct table {
	size_t columns_len;
	struct entry *columns;
};

// https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-03#page-15
static const struct test_vector {
	const char *input;
	uint64_t expected;
} TEST_VECTORS[] = {
	{"", 0xaf63bd4c8601b7df},
	{"a", 0x089be207b544f1e4},
	{"foobar", 0x34531ca7168b8f38},
	{NULL, 0},
};

static const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;
static const uint64_t FNV_PRIME = 0x100000001b3;

// Imagine if this didn't hold...
_Static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

/// Return FNV-1a hash of input
static uint64_t fnv_hash(const size_t data_len, const unsigned char data[data_len])
{
	extern const uint64_t FNV_OFFSET_BASIS;
	extern const uint64_t FNV_PRIME;

	uint64_t hash = FNV_OFFSET_BASIS;

	for (size_t i = 0; i < data_len; ++i) {
		hash ^= data[i];
		hash *= FNV_PRIME;
	}

	return hash;
}

static bool test_fnv_hash(void)
{
	extern const struct test_vector TEST_VECTORS[];

	(void)printf("testing fnv_hash\n");

	const char *input = NULL;
	for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
		const uint64_t expected = TEST_VECTORS[i].expected;
		const uint64_t actual = fnv_hash(strlen(input) + 1, (const unsigned char *)input);
		if (expected != actual) {
			(void)fprintf(stderr, "input: \"%s\", expected: %ld, actual: %ld\n",
				input, expected, actual);
			return false;
		}
	}

	return true;
}

static struct table *table_create(const size_t columns_len)
{
	if ((columns_len & (columns_len - 1)) != 0) {
		(void)fprintf(stderr, "Error: columns_len must be a power of 2\n");
		return NULL;
	}

	struct table *ret = emalloc(sizeof(*ret));
	ret->columns = ecalloc(columns_len, sizeof(*ret->columns));
	ret->columns_len = columns_len;
	return ret;
}

static void table_destroy(struct table *t)
{
	struct entry *curr;
	struct entry *next;

	if (t == NULL) {
		return;
	}

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

static int table_put(struct table *t, const char *key, void *value)
{
	const uint64_t hash = fnv_hash(strlen(key) + 1, (const unsigned char *)key);
	const ptrdiff_t index = (ptrdiff_t)(hash & (uint64_t)(t->columns_len - 1));
	struct entry *curr = t->columns + index;
	struct entry *prev = NULL;

	while (curr != NULL && curr->key != NULL && strcmp(key, curr->key) != 0) {
		prev = curr;
		curr = curr->next;
	}

	// first node
	if (curr != NULL && curr->key == NULL) {
		curr->key = key;
		curr->value = value;
		return 0;
	}

	// existing node
	if (curr != NULL && curr->key != NULL) {
		assert(strcmp(curr->key, key) == 0);
		curr->value = value;
		return 0;
	}

	// new node
	assert(curr == NULL);

	curr = ecalloc(1, sizeof(*curr));

	curr->next = NULL;
	curr->key = key;
	curr->value = value;
	prev->next = curr;

	return 0;
}

static void *table_get(struct table *t, const char *key)
{
	const uint64_t hash = fnv_hash(strlen(key) + 1, (const unsigned char *)key);
	const ptrdiff_t index = (ptrdiff_t)(hash & (uint64_t)(t->columns_len - 1));
	struct entry *curr = t->columns + index;

	while (curr != NULL && strcmp(key, curr->key) != 0) {
		curr = curr->next;
	}

	if (curr == NULL) {
		return NULL;
	}

	return curr->value;
}

int main(void)
{
	int ret = EXIT_FAILURE;

	if (test_fnv_hash() == false) {
		return EXIT_FAILURE;
	}

	const char *key = "foo";
	char *value = "bar";
	struct table *t = table_create(16);

	(void)table_put(t, key, value);
	char *ret_value = table_get(t, key);
	if (ret_value == NULL) {
		goto out_table_destroy;
	}

	(void)printf("%s: %s\n", key, ret_value);

	ret = EXIT_SUCCESS;
out_table_destroy:
	table_destroy(t);
	return ret;
}
