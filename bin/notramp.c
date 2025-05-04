#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "feature.h" // IWYU pragma: keep

#if HAS_NESTED_FUNCTIONS
long f1(void)
{
	long i = 0;
	void f2(void) { ++i; }
	f2();
	f2();
	return i;
}

int main(void)
{
	assert(f1() == 2L);
	return EXIT_SUCCESS;
}
#else
int main(void)
{
	printf("Compiled without support for nested functions\n");
	return EXIT_SUCCESS;
}
#endif
