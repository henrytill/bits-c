#include <omp.h>
#include <stdio.h>

// clang-format off

int main(void)
{
    #pragma omp parallel
    printf("Hello, world.\n");
    return 0;
}

// clang-format on

/* Local Variables: */
/* eval: (c-set-offset 'cpp-macro 0 nil) */
/* End: */
