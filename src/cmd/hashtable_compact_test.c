#include <stdlib.h>

#include "bits.h"
#include "printf.h"

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
run(Table *t, int test)
{
	char const *testname = testcases[test].name;
	int i;
	intptr_t value;
	int rc;
	void *result;

	for(i = 0; testcases[test].toadd[i] != NULL; ++i) {
		char const *key = testcases[test].toadd[i];
		value = i + 1;
		rc = tableput(t, key, (void *)value);
		if(rc != 0) {
			eprintf("FAIL %s: tableput failed for key '%s'\n", testname, key);
			return 0;
		}
	}

	for(i = 0; testcases[test].todelete[i] != NULL; ++i) {
		char const *key = testcases[test].todelete[i];
		rc = tabledel(t, key, NULL);
		if(rc != 0) {
			eprintf("FAIL %s: tabledel failed for key '%s'\n", testname, key);
			return 0;
		}
	}

	for(i = 0; testcases[test].shouldnotexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldnotexist[i];
		result = tableget(t, key);
		if(result != NULL) {
			eprintf("FAIL %s: key '%s' should not exist before compaction\n", testname, key);
			return 0;
		}
	}

	for(i = 0; testcases[test].shouldexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldexist[i];
		result = tableget(t, key);
		if(result == NULL) {
			eprintf("FAIL %s: key '%s' should exist before compaction\n", testname, key);
			return 0;
		}
	}

	tablecompact(t);

	for(i = 0; testcases[test].shouldnotexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldnotexist[i];
		result = tableget(t, key);
		if(result != NULL) {
			eprintf("FAIL %s: key '%s' should not exist after compaction\n", testname, key);
			return 0;
		}
	}

	for(i = 0; testcases[test].shouldexist[i] != NULL; ++i) {
		char const *key = testcases[test].shouldexist[i];
		result = tableget(t, key);
		if(result == NULL) {
			eprintf("FAIL %s: key '%s' should exist after compaction\n", testname, key);
			return 0;
		}
	}

	value = 999;
	rc = tableput(t, "post_compact_key", (void *)value);
	if(rc != 0) {
		eprintf("FAIL %s: tableput failed after compaction\n", testname);
		return 0;
	}

	result = tableget(t, "post_compact_key");
	if(result != (void *)value) {
		eprintf("FAIL %s: tableget failed for post-compaction key\n", testname);
		return 0;
	}

	return 1;
}

int
main(void)
{
	int ret = EXIT_FAILURE;
	Table *t;
	int test;
	int rc;

	t = tablecreate(8);
	if(t == NULL) {
		eprintf("FAIL: tablecreate failed\n");
		return EXIT_FAILURE;
	}

	for(test = 0; testcases[test].name != NULL; ++test) {
		rc = run(t, test);
		if(!rc)
			goto destroyt;

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
