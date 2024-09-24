#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__) && !defined(__clang__)
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#    define HAS_NESTED_FUNCTIONS 1
#  endif
#endif

#if HAS_NESTED_FUNCTIONS
long f1(void) {
  long i = 0;
  void f2(void) { i++; }
  f2();
  f2();
  return i;
}

int main(void) {
  assert(f1() == 2);
  return EXIT_SUCCESS;
}
#else
int main(void) {
  printf("Compiled without support for nested functions\n");
  return EXIT_SUCCESS;
}
#endif
