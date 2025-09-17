#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bits.h"

/* https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-03#page-15 */
static struct test_vector {
	char const *input;
	uint64_t expected;
} const TEST_VECTORS[] = {
	{"", 0xaf63bd4c8601b7df},
	{"a", 0x089be207b544f1e4},
	{"foobar", 0x34531ca7168b8f38},
	{NULL, 0},
};

static bool
check(char const *input, uint64_t const expected, uint64_t const actual)
{
	if(expected == actual) {
		return true;
	}
	eprintf("input: \"%s\", expected: %" PRIu64 ", actual: %" PRIu64 "\n", input, expected, actual);
	return false;
}

int
main(void)
{
	size_t i;
	char const *input;
	uint64_t expected, actual;

	for(i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
		if(input == NULL) {
			return EXIT_FAILURE;
		}
		expected = TEST_VECTORS[i].expected;
		actual = fnv_hash(strlen(input) + 1, (unsigned char const *)input);
		if(!check(input, expected, actual)) {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
