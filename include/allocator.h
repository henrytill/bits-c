#ifndef C_BITS_ALLOCATOR_H
#define C_BITS_ALLOCATOR_H

#define ALLOCATOR_DEFINE(T)                                                \
  typedef struct T##_allocator T##_allocator;                              \
                                                                           \
  struct T##_allocator {                                                   \
    size_t capacity;                                                       \
    size_t count;                                                          \
    T items[];                                                             \
  };                                                                       \
                                                                           \
  T##_allocator *T##_allocator_create(size_t capacity) {                   \
    T##_allocator *ret = calloc(1, sizeof(*ret) + (capacity * sizeof(T))); \
    if (ret == NULL) {                                                     \
      (void)fprintf(stderr, "%s: calloc failed\n", __func__);              \
      exit(EXIT_FAILURE);                                                  \
    }                                                                      \
    ret->capacity = capacity;                                              \
    ret->count = 0;                                                        \
    return ret;                                                            \
  }                                                                        \
                                                                           \
  void T##_allocator_destroy(T##_allocator *alloc) {                       \
    free(alloc);                                                           \
  }                                                                        \
                                                                           \
  T *T##_alloc(T##_allocator *alloc) {                                     \
    if (alloc->count >= alloc->capacity) {                                 \
      (void)fprintf(stderr, "%s: capacity exceeded\n", __func__);          \
      exit(EXIT_FAILURE);                                                  \
    }                                                                      \
    return &alloc->items[alloc->count++];                                  \
  }

#endif
