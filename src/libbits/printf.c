#include <stdarg.h>
#include <stdio.h>

#include "bits.h"

void
eprintf(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
}

#ifdef DEBUG
void
debug_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vprintf(fmt, args);
	va_end(args);
}
#else
void
debug_printf(const char *fmt, ...)
{
	(void)fmt;
}
#endif

#ifdef DEBUG
void
debug_eprintf(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
}
#else
void
debug_eprintf(char const *fmt, ...)
{
	(void)fmt;
}
#endif
