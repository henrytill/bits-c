#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "macro.h"

#define TEST(e)                                         \
    if (!(e)) {                                         \
        eprintf("%s:%d: %s\n", __FILE__, __LINE__, #e); \
        exit(EXIT_FAILURE);                             \
    }

// https://boringssl.googlesource.com/boringssl/+/master/crypto/base64/base64_test.cc#49
#define TEST_VECTORS_ENTRIES \
    X("", "")                \
    X("f", "Zg==")           \
    X("fo", "Zm8=")          \
    X("foo", "Zm9v")         \
    X("foob", "Zm9vYg==")    \
    X("fooba", "Zm9vYmE=")   \
    X("foobar", "Zm9vYmFy")

#define BASE64_STRLEN(s) (((strlen((s)) + 2) / 3) * 4)

#define X(s, b) STATIC_ASSERT(BASE64_STRLEN(s) == strlen(b));
TEST_VECTORS_ENTRIES
#undef X

static struct test_vector {
    char const *input;
    size_t const input_len;
    char const *base64;
    size_t const base64_len;
} const TEST_VECTORS[] = {
#define X(s, b) {(s), strlen((s)), (b), strlen((b))},
    TEST_VECTORS_ENTRIES
#undef X
    {NULL, 0, NULL, 0},
};

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define LAST_ENTRY      (ARRAY_SIZE(TEST_VECTORS) - 2)
#define INPUT_LEN_MAX   (TEST_VECTORS[LAST_ENTRY].input_len)
#define BASE64_LEN_MAX  (TEST_VECTORS[LAST_ENTRY].base64_len)

#define ARRAY_CLEAR(arr) memset((arr), 0, sizeof((arr))) // NOLINT(clang-analyzer-security.*)

static size_t base64_encode(size_t const in_strlen, char const in[in_strlen + 1], char *out) {
    return (size_t)EVP_EncodeBlock(
        (unsigned char *)out,
        (unsigned char const *)in,
        (int)in_strlen
    );
}

static size_t base64_decode(size_t const in_strlen, char const in[in_strlen + 1], char *out) {
    return (size_t)EVP_DecodeBlock(
        (unsigned char *)out,
        (unsigned char const *)in,
        (int)in_strlen
    );
}

int main(void) {
    char const *input = NULL;
    size_t input_len = 0;
    char const *expected_base64 = NULL;
    size_t expected_base64_len = 0;

    size_t codec_len = 0;

    char actual_base64[BASE64_LEN_MAX + 1];
    char output[INPUT_LEN_MAX + 1];

    for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
        ARRAY_CLEAR(actual_base64);
        ARRAY_CLEAR(output);

        input_len = TEST_VECTORS[i].input_len;
        expected_base64 = TEST_VECTORS[i].base64;
        expected_base64_len = TEST_VECTORS[i].base64_len;

        printf("input: %s\n", input);
        printf("input_len: %ld\n", input_len);
        printf("expected_base64: %s\n", expected_base64);
        printf("expected_base64_len: %ld\n", expected_base64_len);

        codec_len = base64_encode(input_len, input, actual_base64);

        TEST(codec_len == strlen(actual_base64));
        TEST(codec_len == expected_base64_len);
        TEST(strcmp(actual_base64, expected_base64) == 0);

        codec_len = base64_decode(codec_len, actual_base64, output);

        printf("codec_len: %ld\n", codec_len);
        printf("\n");

        TEST(input_len == strlen(output));
        TEST(strcmp(output, input) == 0);
    }

    return EXIT_SUCCESS;
}
