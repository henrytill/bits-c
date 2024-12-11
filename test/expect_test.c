#include "expect.h"

int main(void) {
  char *a = "hello, world!";
  EXPECT(a, {"hello, world!"});

  char *b = "goodbye, world";
  EXPECT(b, {"goodbye, world"});

  return EXIT_SUCCESS;
}
