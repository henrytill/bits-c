#include <stdarg.h>
#include <stdio.h>

static __inline__ void eprintf(char const *fmt, ...);

#ifdef DEBUG
static __inline__ void debug_printf(char const *fmt, ...);
static __inline__ void debug_eprintf(char const *fmt, ...);
#else
static __inline__ void debug_printf(char const *fmt, ...);
static __inline__ void debug_eprintf(char const *fmt, ...);
#endif

static __inline__ void
eprintf(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
}

#ifdef DEBUG

static __inline__ void
debug_printf(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vprintf(fmt, args);
	va_end(args);
}

static __inline__ void
debug_eprintf(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
}

#else

static __inline__ void
debug_printf(char const *fmt, ...)
{
	(void)fmt;
}

static __inline__ void
debug_eprintf(char const *fmt, ...)
{
	(void)fmt;
}

#endif
