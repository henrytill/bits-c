#include "bits.h"
#include "macro.h"

struct Message const expected[] = {
  {Tsome, 42},
  {Tsome, 0},
  {Tsome, 1},
  {Tsome, 2},
};

size_t const expectedlen = NELEM(expected);
