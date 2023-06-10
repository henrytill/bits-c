#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "prelude.h"

#define STATIC_ASSERT(e) _Static_assert((e), #e)

#define BASE64_STRLEN(s) (((strlen((s)) + 2) / 3) * 4)

#define TEST(e, l)                                                          \
	if (!(e)) {                                                         \
		(void)fprintf(stderr, "%s:%d: %s", __FILE__, __LINE__, #e); \
		goto l;                                                     \
	}

// https://boringssl.googlesource.com/boringssl/+/master/crypto/base64/base64_test.cc#49
#define TEST_VECTORS_ENTRIES   \
	X("", "")              \
	X("f", "Zg==")         \
	X("fo", "Zm8=")        \
	X("foo", "Zm9v")       \
	X("foob", "Zm9vYg==")  \
	X("fooba", "Zm9vYmE=") \
	X("foobar", "Zm9vYmFy")

#define X(s, b) STATIC_ASSERT(BASE64_STRLEN(s) == strlen(b));
TEST_VECTORS_ENTRIES
#undef X

static const struct test_vector {
	const char *input;
	const size_t input_len;
	const char *base64;
	const size_t base64_len;
} TEST_VECTORS[] = {
#define X(s, b) {(s), strlen((s)), (b), strlen((b))},
	TEST_VECTORS_ENTRIES
#undef X
	{NULL, 0, NULL, 0},
};

static size_t base64_encode(const size_t in_len, const char in[in_len + 1], char *out)
{
	return (size_t)EVP_EncodeBlock((unsigned char *)out, (const unsigned char *)in, (int)in_len);
}

static size_t base64_decode(const size_t in_len, const char in[in_len + 1], char *out)
{
	return (size_t)EVP_DecodeBlock((unsigned char *)out, (const unsigned char *)in, (int)in_len);
}

int main(void)
{
	extern const struct test_vector TEST_VECTORS[];

	const char *input = NULL;
	size_t input_len = 0;
	const char *expected_base64 = NULL;
	size_t expected_base64_len = 0;

	size_t codec_len = 0;

	char *actual_base64 = NULL;
	size_t actual_base64_len = 0;
	char *output = NULL;
	size_t output_len = 0;

	for (size_t i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
		input_len = TEST_VECTORS[i].input_len;
		expected_base64 = TEST_VECTORS[i].base64;
		expected_base64_len = TEST_VECTORS[i].base64_len;

		(void)printf("input: %s\n", input);
		(void)printf("input_len: %ld\n", input_len);
		(void)printf("expected_base64: %s\n", expected_base64);
		(void)printf("expected_base64_len: %ld\n", expected_base64_len);

		actual_base64 = ecalloc(expected_base64_len + 1, sizeof(*actual_base64));
		codec_len = base64_encode(input_len, input, actual_base64);
		actual_base64_len = strlen(actual_base64);

		TEST(codec_len == actual_base64_len, out_free_actual_base64);

		TEST(actual_base64_len == expected_base64_len, out_free_actual_base64);

		TEST(strcmp(actual_base64, expected_base64) == 0, out_free_actual_base64);

		output = ecalloc(input_len + 1, sizeof(*output));
		codec_len = base64_decode(codec_len, actual_base64, output);
		output_len = strlen(output);

		(void)printf("codec_len: %ld\n", codec_len);
		(void)printf("\n");

		TEST(output_len == input_len, out_free_output);

		TEST(strcmp(output, input) == 0, out_free_output);

		free(output);
		free(actual_base64);
	}

	return EXIT_SUCCESS;

out_free_output:
	free(output);
out_free_actual_base64:
	free(actual_base64);
	return EXIT_FAILURE;
}
