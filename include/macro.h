#ifndef C_BITS_MACRO_H
#define C_BITS_MACRO_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h> // IWYU pragma: keep

#ifndef NDEBUG
#	define DEBUG
#endif

// General

#define eprintf(...) (void)fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#	define debug_printf(fmt, ...)          printf(fmt, ##__VA_ARGS__)
#	define debug_fprintf(stream, fmt, ...) (void)fprintf(stream, fmt, ##__VA_ARGS__)
#	define debug_eprintf(...)              (void)fprintf(stderr, __VA_ARGS__)
#else
#	define debug_printf(fmt, ...)
#	define debug_fprintf(stream, fmt, ...)
#	define debug_eprintf(...)
#endif

// OOP

#define TYPEOF_MEMBER(type, member) typeof(((type *)0)->member)

#define SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define CONTAINER_OF_(ptr, type, member) ({                                                                        \
	char *mptr = (char *)(ptr);                                                                                \
	static_assert(SAME_TYPE(*(ptr), ((type *)0)->member) || SAME_TYPE(*(ptr), void), "pointer type mismatch"); \
	((type *)(mptr - offsetof(type, member)));                                                                 \
})

#define CONTAINER_OF(ptr, type, member)                                                   \
	_Generic(                                                                         \
		ptr,                                                                      \
		typeof(*(ptr)) const *: ((type const *)CONTAINER_OF_(ptr, type, member)), \
		default: ((type *)CONTAINER_OF_(ptr, type, member)))

#define SEND(obj, method, ...) ({                  \
	typeof(obj) receiver = (obj);              \
	receiver->method(receiver, ##__VA_ARGS__); \
})

// Cleanup

#define AT_EXIT(func)                                  \
	if(atexit(func) != 0) {                        \
		eprintf("atexit(%s) failed\n", #func); \
		exit(EXIT_FAILURE);                    \
	}

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)                                      \
	static inline void func##p(type *p) /* NOLINT(bugprone-macro-parentheses) */ \
	{                                                                            \
		if(*p) {                                                             \
			func(*p);                                                    \
			debug_printf("%s(*%p)\n", #func, (void *)p);                 \
		}                                                                    \
	}

// Misc

/// Returns 1 if x is a power of 2
#define ISPOW2(x) (((x) & ((x) - 1)) == 0)

#define STATIC_ASSERT(e) _Static_assert((e), #e)

#endif // C_BITS_MACRO_H
