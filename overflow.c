#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int add1(int x)
{
	return x + 1;
}

int main(void)
{
	int ret = add1(INT_MAX);
	(void)printf("ret: %d", ret);
	return EXIT_SUCCESS;
}