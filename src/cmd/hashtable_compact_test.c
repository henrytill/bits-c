#include <stdlib.h>

#include "bits.h"

static struct {
	char const *name;
	char const *toadd[8];
	char const *todelete[8];
	char const *shouldexist[8];
	char const *shouldnotexist[8];
} const testcases[] = {
	{
		"simple_deletion",
		{"key1", "key2", "key3", "key4", NULL},
		{"key2", "key4", NULL},
		{"key1", "key3", NULL},
		{"key2", "key4", NULL},
	},
	{
		"delete_all_then_readd",
		{"a", "b", "c", NULL},
		{"a", "b", "c", NULL},
		{NULL},
		{"a", "b", "c", NULL},
	},
	{
		"embedded_entry_deletion",
		{"test1", "test2", "test3", NULL},
		{"test1", NULL},
		{"test2", "test3", NULL},
		{"test1", NULL},
	},
	{
		"chain_cleanup",
		{"x1", "x2", "x3", "x4", "x5", NULL},
		{"x2", "x4", NULL},
		{"x1", "x3", "x5", NULL},
		{"x2", "x4", NULL},
	},
	{
		"no_deletions",
		{"keep1", "keep2", "keep3", NULL},
		{NULL},
		{"keep1", "keep2", "keep3", NULL},
		{NULL},
	},
	{NULL, {NULL}, {NULL}, {NULL}, {NULL}},
};

static int
run(struct Table *t, int test)
{
	char const *testname = testcases[test].name;
	size_t i;

	/* Add entries */
	for(i = 0; testcases[test].toadd[i] != NULL; ++i) {
		char const *key = testcases[test].toadd[i];
		int value = (int)(i + 1);
		if(tableput(t, key, (void *)(intptr_t)value) != 0) {
			eprintf("FAIL %s: tableput failed for key '%s'\n", testname, key);
			return 0;
		}
	}

	/* Delete entries */
	for(i = 0; testcases[test].todelete[i] != NULL; ++i) {
		char const *key = testcases[test].todelete[i];
		if(tabledel(t, key, NULL) != 0) {
			eprintf("FAIL %s: tabledel failed for key '%s'\n", testname, key);
			return 0;
		}
	}

	/* Verify deleted entries are gone before compaction */
	for(i = 0; testcases[test].shouldnotexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldnotexist[i];
		if(tableget(t, key) != NULL) {
			eprintf("FAIL %s: key '%s' should not exist before compaction\n", testname, key);
			return 0;
		}
	}

	/* Verify remaining entries exist before compaction */
	for(i = 0; testcases[test].shouldexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldexist[i];
		if(tableget(t, key) == NULL) {
			eprintf("FAIL %s: key '%s' should exist before compaction\n", testname, key);
			return 0;
		}
	}

	/* Compact the table */
	tablecompact(t);

	/* Verify deleted entries are still gone after compaction */
	for(i = 0; testcases[test].shouldnotexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldnotexist[i];
		if(tableget(t, key) != NULL) {
			eprintf("FAIL %s: key '%s' should not exist after compaction\n", testname, key);
			return 0;
		}
	}

	/* Verify remaining entries still exist after compaction */
	for(i = 0; testcases[test].shouldexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldexist[i];
		if(tableget(t, key) == NULL) {
			eprintf("FAIL %s: key '%s' should exist after compaction\n", testname, key);
			return 0;
		}
	}

	/* Test that we can still add new entries after compaction */
	if(tableput(t, "post_compact_key", (void *)999) != 0) {
		eprintf("FAIL %s: tableput failed after compaction\n", testname);
		return 0;
	}

	if(tableget(t, "post_compact_key") != (void *)999) {
		eprintf("FAIL %s: tableget failed for post-compaction key\n", testname);
		return 0;
	}

	return 1;
}

int
main(void)
{
	int ret = EXIT_FAILURE;
	struct Table *t;
	int test;

	t = tablecreate(8);
	if(t == NULL) {
		eprintf("FAIL: tablecreate failed\n");
		return EXIT_FAILURE;
	}

	for(test = 0; testcases[test].name != NULL; ++test) {
		if(!run(t, test))
			goto destroyt;

		/* Clear table for next test */
		tabledestroy(t, NULL);
		t = tablecreate(8);
		if(t == NULL) {
			eprintf("FAIL: tablecreate failed during test cleanup\n");
			goto destroyt;
		}
	}

	ret = EXIT_SUCCESS;

destroyt:
	tabledestroy(t, NULL);
	return ret;
}
