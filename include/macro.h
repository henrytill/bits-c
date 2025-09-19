#pragma once

#include <stddef.h>
#include <string.h> /* IWYU pragma: keep */

#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

#define SEND(obj, method) ((obj)->method(obj))

#define NELEM(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CLEAR(x) memset(x, 0, sizeof(x))

#define ISPOW2(x) (((x) & ((x) - 1)) == 0)
