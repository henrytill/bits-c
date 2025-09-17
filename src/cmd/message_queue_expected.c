#include "bits.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

struct message const EXPECTED[] = {
	{.tag = MSG_TAG_SOME, .value = 42},
	{.tag = MSG_TAG_SOME, .value = 0},
	{.tag = MSG_TAG_SOME, .value = 1},
	{.tag = MSG_TAG_SOME, .value = 2},
};

size_t const EXPECTED_LEN = ARRAY_LENGTH(EXPECTED);
