#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fnv.h"

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

static bool check(const char *input, const uint64_t expected, const uint64_t actual)
{
    if (expected == actual) { return true; }
    (void)fprintf(stderr, "input: \"%s\", expected: %" PRIu64 ", actual: %" PRIu64 "\n",
                  input, expected, actual);
    return false;
}

int main(void)
{
    extern const struct test_vector TEST_VECTORS[];

    const char *input = NULL;
    for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
        const uint64_t expected = TEST_VECTORS[i].expected;
        const uint64_t actual = fnv_hash(strlen(input) + 1, (const unsigned char *)input);
        if (!check(input, expected, actual)) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
