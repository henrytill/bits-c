#ifndef C_BITS_MACRO
#define C_BITS_MACRO

#include <stddef.h>
#include <stdio.h>

#ifndef NDEBUG
#  define DEBUG
#endif

// C2X compatibility

#ifndef static_assert
#  define static_assert _Static_assert
#endif

// General

#define eprintf(...) (void)fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#  define debug_printf(fmt, ...)          printf(fmt, ##__VA_ARGS__)
#  define debug_fprintf(stream, fmt, ...) (void)fprintf(stream, fmt, ##__VA_ARGS__)
#  define debug_eprintf(...)              (void)fprintf(stderr, __VA_ARGS__)
#else
#  define debug_printf(fmt, ...)          ({})
#  define debug_fprintf(stream, fmt, ...) ({})
#endif

// OOP

#define TYPEOF_MEMBER(type, member) typeof(((type *)0)->member)

#define SAME_TYPE(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define CONTAINER_OF_(ptr, type, member) ({                                                                  \
  char *__mptr = (char *)(ptr);                                                                              \
  static_assert(SAME_TYPE(*(ptr), ((type *)0)->member) || SAME_TYPE(*(ptr), void), "pointer type mismatch"); \
  ((type *)(__mptr - offsetof(type, member)));                                                               \
})

#define CONTAINER_OF(ptr, type, member)                                       \
  _Generic(                                                                   \
    ptr,                                                                      \
    const typeof(*(ptr)) *: ((const type *)CONTAINER_OF_(ptr, type, member)), \
    default: ((type *)CONTAINER_OF_(ptr, type, member)))

#define SEND(obj, method, ...) ({      \
  typeof(obj) __obj = (obj);           \
  __obj->method(__obj, ##__VA_ARGS__); \
})

// Cleanup

#define AT_EXIT(func) ({                   \
  if (atexit(func) != 0) {                 \
    eprintf("atexit(%s) failed\n", #func); \
    exit(EXIT_FAILURE);                    \
  }                                        \
})

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)                                  \
  static inline void func##p(type *p) { /* NOLINT(bugprone-macro-parentheses) */ \
    if (*p) {                                                                    \
      func(*p);                                                                  \
      debug_printf("%s(*%p)\n", #func, (void *)p);                               \
    }                                                                            \
  }

// Misc

/// Returns 1 if x is a power of 2
#define ISPOW2(x) (((x) & ((x)-1)) == 0)

#endif // C_BITS_MACRO
