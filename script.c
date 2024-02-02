#if 0
set -e
cc "$0"
exec ./a.out "$@"
#endif

/*
 * To run:
 *
 *     sh script.c
 */
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("Hello, world!\n");
    return 0;
}
