#include <assert.h>
#include <stdlib.h>

#include <openssl/evp.h>

#include "bits.h"

#define TEST(e)                                                 \
	if(!(e)) {                                              \
		eprintf("%s:%d: %s\n", __FILE__, __LINE__, #e); \
		exit(EXIT_FAILURE);                             \
	}

#define BASE64_STRLEN(s) (((strlen((s)) + 2) / 3) * 4)

static struct {
	char const *input;
	size_t const inputlen;
	char const *base64;
	size_t const base64len;
} const vectors[] = {
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
base64_encode(size_t const inlen, char const *in, char *out)
{
	return (size_t)EVP_EncodeBlock((unsigned char *)out, (unsigned char const *)in, (int)inlen);
}

static size_t
base64_decode(size_t const inlen, char const *in, char *out)
{
	return (size_t)EVP_DecodeBlock((unsigned char *)out, (unsigned char const *)in, (int)inlen);
}

int
main(void)
{
	size_t i;
	char const *input, *expected;
	size_t inputlen, expectedlen, codeclen;
	char actual[BASE64_LEN_MAX + 1];
	char output[INPUT_LEN_MAX + 1];

	for(i = 0; (input = vectors[i].input) != NULL; ++i) {
		CLEAR(actual);
		CLEAR(output);

		inputlen = vectors[i].inputlen;
		expected = vectors[i].base64;
		expectedlen = vectors[i].base64len;

		printf("input: %s\n", input);
		printf("input_len: %ld\n", inputlen);
		printf("expected: %s\n", expected);
		printf("expected_len: %ld\n", expectedlen);

		codeclen = base64_encode(inputlen, input, actual);

		TEST(codeclen == strlen(actual));
		TEST(codeclen == expectedlen);
		TEST(strcmp(actual, expected) == 0);

		codeclen = base64_decode(codeclen, actual, output);

		printf("codec_len: %ld\n", codeclen);
		printf("\n");

		TEST(inputlen == strlen(output));
		TEST(strcmp(output, input) == 0);
	}

	return EXIT_SUCCESS;
}
