#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <openssl/evp.h>

#include "bits.h"

#define TEST(e)                                                 \
	if(!(e)) {                                              \
		eprintf("%s:%d: %s\n", __FILE__, __LINE__, #e); \
		exit(EXIT_FAILURE);                             \
	}

#define BASE64_STRLEN(s) (((strlen((s)) + 2) / 3) * 4)

static struct test_vector {
	char const *input;
	size_t const input_len;
	char const *base64;
	size_t const base64_len;
} const TEST_VECTORS[] = {
	{"", 0, "", 0},
	{"f", 1, "Zg==", 4},
	{"fo", 2, "Zm8=", 4},
	{"foo", 3, "Zm9v", 4},
	{"foob", 4, "Zm9vYg==", 8},
	{"fooba", 5, "Zm9vYmE=", 8},
	{"foobar", 6, "Zm9vYmFy", 8},
	{NULL, 0, NULL, 0},
};

#define INPUT_LEN_MAX  6 /* longest input is "foobar" */
#define BASE64_LEN_MAX 8 /* longest base64 is "Zm9vYmFy" */

static size_t
base64_encode(size_t const in_strlen, char const *in, char *out)
{
	return (size_t)EVP_EncodeBlock((unsigned char *)out, (unsigned char const *)in, (int)in_strlen);
}

static size_t
base64_decode(size_t const in_strlen, char const *in, char *out)
{
	return (size_t)EVP_DecodeBlock((unsigned char *)out, (unsigned char const *)in, (int)in_strlen);
}

int
main(void)
{
	size_t i;
	char const *input, *expected;
	size_t input_len, expected_len, codec_len;
	char actual[BASE64_LEN_MAX + 1];
	char output[INPUT_LEN_MAX + 1];

	for(i = 0; (input = TEST_VECTORS[i].input) != NULL; ++i) {
		CLEAR(actual);
		CLEAR(output);

		input_len = TEST_VECTORS[i].input_len;
		expected = TEST_VECTORS[i].base64;
		expected_len = TEST_VECTORS[i].base64_len;

		printf("input: %s\n", input);
		printf("input_len: %ld\n", input_len);
		printf("expected: %s\n", expected);
		printf("expected_len: %ld\n", expected_len);

		codec_len = base64_encode(input_len, input, actual);

		TEST(codec_len == strlen(actual));
		TEST(codec_len == expected_len);
		TEST(strcmp(actual, expected) == 0);

		codec_len = base64_decode(codec_len, actual, output);

		printf("codec_len: %ld\n", codec_len);
		printf("\n");

		TEST(input_len == strlen(output));
		TEST(strcmp(output, input) == 0);
	}

	return EXIT_SUCCESS;
}
