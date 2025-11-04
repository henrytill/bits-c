#pragma once

#include <stdarg.h>
#include <stdio.h>

static __inline__ void debugprintf(char const *fmt, ...);
static __inline__ void eprintf(char const *fmt, ...);

#ifndef NDEBUG
static __inline__ void debugprintf(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)vprintf(fmt, args);
    va_end(args);
}
#else
static __inline__ void debugprintf(char const *fmt, ...)
{
    (void)fmt;
}
#endif

static __inline__ void eprintf(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)vfprintf(stderr, fmt, args);
    va_end(args);
}
