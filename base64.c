#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "prelude.h"

// https://boringssl.googlesource.com/boringssl/+/master/crypto/base64/base64_test.cc#49
static const struct test_vector {
    const char *input;
    const char *base64;
} TEST_VECTORS[] = {
    {"", ""},
    {"f", "Zg=="},
    {"fo", "Zm8="},
    {"foo", "Zm9v"},
    {"foob", "Zm9vYg=="},
    {"fooba", "Zm9vYmE="},
    {"foobar", "Zm9vYmFy"},
    {NULL, NULL},
};

static size_t base64_strlen(const char *in)
{
    return ((strlen(in) + 2) / 3) * 4;
}

static long base64_encode(const unsigned char *in, int in_len,
                          unsigned char *out)
{
    return EVP_EncodeBlock(out, in, in_len);
}

static long base64_decode(const unsigned char *in, int in_len,
                          unsigned char *out)
{
    return EVP_DecodeBlock(out, in, in_len);
}

int main(void)
{
    extern const struct test_vector TEST_VECTORS[];

    long codec_len = 0L;
    const char *input = NULL;
    for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
        const size_t input_len = strlen(input);

        printf("input: %s\n", input);
        printf("input_len: %ld\n", input_len);

        const size_t input_base64_len = base64_strlen(input);
        const char *expected = TEST_VECTORS[i].base64;
        const size_t expected_len = strlen(expected);

        if (expected_len != input_base64_len) {
            (void)fprintf(stderr, "expected_len: %ld, input_base64_len: %ld\n",
                          expected_len, input_base64_len);
            return EXIT_FAILURE;
        }

        char *actual_base64 = ecalloc(input_base64_len + 1, sizeof(*actual_base64));
        codec_len = base64_encode((const unsigned char *)input,
                                  (int)input_len,
                                  (unsigned char *)actual_base64);
        const size_t actual_base64_len = strlen(actual_base64);
        assert((size_t)codec_len == actual_base64_len);

        if (actual_base64_len != input_base64_len) {
            (void)fprintf(stderr, "input: %s, input_base64_len: %ld, actual_base64: %s, actual_base64_len: %ld\n",
                          input, input_base64_len, actual_base64, actual_base64_len);
            free(actual_base64);
            return EXIT_FAILURE;
        }

        if (strcmp(actual_base64, expected) != 0) {
            (void)fprintf(stderr, "input: %s, actual_base64: %s, base64: %s\n",
                          input, actual_base64, expected);
            free(actual_base64);
            return EXIT_FAILURE;
        }

        char *actual_input = ecalloc(input_len + 1, sizeof(*actual_input));
        codec_len = base64_decode((const unsigned char *)actual_base64,
                                  (int)codec_len,
                                  (unsigned char *)actual_input);
        const size_t actual_input_len = strlen(actual_input);
        printf("codec_len: %ld\n", codec_len);
        printf("\n");

        if (actual_input_len != input_len) {
            (void)fprintf(stderr, "input: %s, input_len: %ld, actual_input: %s, actual_input_len: %ld\n",
                          input, input_len, actual_input, actual_input_len);
            free(actual_input);
            free(actual_base64);
            return EXIT_FAILURE;
        }

        if (strcmp(actual_input, input) != 0) {
            (void)fprintf(stderr, "input: %s, actual_input: %s",
                          input, actual_input);
            free(actual_input);
            free(actual_base64);
            return EXIT_FAILURE;
        }

        free(actual_input);
        free(actual_base64);
    }

    return EXIT_SUCCESS;
}
