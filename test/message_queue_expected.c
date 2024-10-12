#include "message_queue.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

const struct message EXPECTED[] = {
    {.tag = MSG_TAG_SOME, .value = 42},
    {.tag = MSG_TAG_SOME, .value = 0},
    {.tag = MSG_TAG_SOME, .value = 1},
    {.tag = MSG_TAG_SOME, .value = 2},
};

const size_t EXPECTED_LEN = ARRAY_LENGTH(EXPECTED);
