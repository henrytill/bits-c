#include "bits.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

struct message const EXPECTED[] = {
	{MSG_TAG_SOME, 42},
	{MSG_TAG_SOME, 0},
	{MSG_TAG_SOME, 1},
	{MSG_TAG_SOME, 2},
};

size_t const EXPECTED_LEN = ARRAY_LENGTH(EXPECTED);
