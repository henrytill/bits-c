#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "bits.h"
#include "printf.h"

/* https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-03#page-15 */
static struct {
  char const *input;
  uint64_t expected;
} const vectors[] = {
  {"", 0xaf63bd4c8601b7df},
  {"a", 0x089be207b544f1e4},
  {"foobar", 0x34531ca7168b8f38},
  {NULL, 0},
};

static int check(char const *input, uint64_t const expected, uint64_t const actual) {
  if (expected == actual)
    return 1;

  eprintf("input: \"%s\", expected: %" PRIu64 ", actual: %" PRIu64 "\n", input, expected, actual);
  return 0;
}

int main(void) {
  size_t i;
  char const *input;
  uint64_t expected, actual;

  for (i = 0; (input = vectors[i].input) != NULL; ++i) {
    if (input == NULL)
      return EXIT_FAILURE;

    expected = vectors[i].expected;
    actual = fnv(strlen(input) + 1, (unsigned char const *)input);
    if (!check(input, expected, actual))
      return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
