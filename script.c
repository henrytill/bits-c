#if 0
set -e
cc "$0"
exec ./a.out "$@"
#endif

#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello, world!\n");
    return 0;
}
