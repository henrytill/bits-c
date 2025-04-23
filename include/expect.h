#ifndef C_BITS_EXPECT_H
#define C_BITS_EXPECT_H

#include <stdlib.h>
#include <string.h>

#ifndef EXPECT_DIFF_PROGRAM
#  define EXPECT_DIFF_PROGRAM "/usr/bin/diff"
#endif

int expect_getpromote(void);

int expect_insert(char const *filename, int lineno, char const *toinsert, int promote);

#define EXPECT(expected_, actual_)                                     \
  do {                                                                 \
    char const *a_[1] = actual_;                                       \
    if (strcmp(expected_, a_[0]) == 0) {                               \
      break;                                                           \
    }                                                                  \
    expect_insert(__FILE__, __LINE__, expected_, expect_getpromote()); \
    exit(EXIT_FAILURE);                                                \
  } while (0)

#endif // C_BITS_EXPECT_H
