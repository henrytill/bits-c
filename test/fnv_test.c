#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fnv.h"
#include "macro.h"

// https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-03#page-15
static struct test_vector {
	char const *input;
	uint64_t    expected;
} const TEST_VECTORS[] = {
	{"", 0xaf63bd4c8601b7df},
	{"a", 0x089be207b544f1e4},
	{"foobar", 0x34531ca7168b8f38},
	{NULL, 0},
};

static bool check(char const input[static 1], uint64_t const expected, uint64_t const actual)
{
	if (expected == actual) {
		return true;
	}
	eprintf("input: \"%s\", expected: %" PRIu64 ", actual: %" PRIu64 "\n", input, expected, actual);
	return false;
}

int main(void)
{
	char const *input = NULL;
	for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
		if (input == NULL) {
			return EXIT_FAILURE;
		}
		uint64_t const expected = TEST_VECTORS[i].expected;
		uint64_t const actual   = fnv_hash(strlen(input) + 1, (unsigned char const *)input);
		if (!check(input, expected, actual)) {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
