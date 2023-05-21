#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#include "prelude.h"

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
    long codec_len = 0L;
    const char *const input = "foobar";
    const size_t input_len = strlen(input);
    const size_t input_base64_len = base64_strlen(input);

    printf("input: %s\n", input);
    printf("strlen(input): %ld\n", input_len);
    printf("base64_len: %ld\n\n", input_base64_len);

    char *base64 = emalloc(input_base64_len + 1);
    codec_len = base64_encode((const unsigned char *)input,
                              (int)input_len,
                              (unsigned char *)base64);

    printf("len: %ld\n", codec_len);
    printf("strlen(base64): %ld\n", strlen(base64));
    printf("base64: %s\n\n", base64);

    char *output = emalloc(input_len + 1);
    codec_len = base64_decode((const unsigned char *)base64,
                              (int)codec_len,
                              (unsigned char *)output);

    printf("len: %ld\n", codec_len);
    printf("strlen(output): %ld\n", strlen(output));
    printf("output: %s\n", output);

    if (strcmp(input, output) != 0) {
        (void)fprintf(stderr, "input != output\n");
        free(output);
        free(base64);
        return EXIT_FAILURE;
    }

    free(output);
    free(base64);
    return EXIT_SUCCESS;
}
