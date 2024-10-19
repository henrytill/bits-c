#ifndef C_BITS_EXPECT_H
#define C_BITS_EXPECT_H

#include <stdlib.h>
#include <string.h>

#ifndef EXPECT_DIFF_PROGRAM
#    define EXPECT_DIFF_PROGRAM "/usr/bin/diff"
#endif

int expect_getpromote(void);

int expect_insert(const char *filename, int lineno, const char *toinsert, int promote);

#define EXPECT(_expected, _actual)                                         \
    do {                                                                   \
        const char *__a[] = _actual;                                       \
        if (strcmp(_expected, __a[0]) == 0) {                              \
            break;                                                         \
        }                                                                  \
        expect_insert(__FILE__, __LINE__, _expected, expect_getpromote()); \
        exit(EXIT_FAILURE);                                                \
    } while (0)

#endif
