#ifndef C_BITS_FEATURE_H
#define C_BITS_FEATURE_H

#if defined(__GNUC__) && !defined(__clang__)
#    if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#        define HAS_NESTED_FUNCTIONS 1
#    endif
#endif

#ifdef __clang__
#    if __has_extension(blocks)
#        define HAS_BLOCKS 1
#    endif
#endif

#endif // C_BITS_FEATURE_H
