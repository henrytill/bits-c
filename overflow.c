#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int add1(const int x)
{
    return x + 1;
}

int main(void)
{
    int ret = add1(INT_MAX);
    printf("ret: %d", ret);
    return EXIT_SUCCESS;
}
