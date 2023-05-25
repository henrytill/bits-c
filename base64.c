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
    int ret = EXIT_FAILURE;
    long codec_len = 0L;
    const char *const input = "foobar";
    const size_t input_len = strlen(input);
    const size_t input_base64_len = base64_strlen(input);

    printf("input: %s\n", input);

    char *base64 = emalloc(input_base64_len + 1);
    codec_len = base64_encode((const unsigned char *)input,
                              (int)input_len,
                              (unsigned char *)base64);

    printf("base64: %s\n", base64);

    if (strlen(base64) != input_base64_len) {
        (void)fprintf(stderr, "strlen(base64) != input_base_64_len");
        goto out_free_base64;
    }

    char *output = emalloc(input_len + 1);
    codec_len = base64_decode((const unsigned char *)base64,
                              (int)codec_len,
                              (unsigned char *)output);

    printf("output: %s\n", output);

    if (strlen(output) != (size_t)codec_len) {
        (void)fprintf(stderr, "strlen(output) != codec_len");
        goto out_free_output;
    }

    if (strcmp(input, output) != 0) {
        (void)fprintf(stderr, "input != output\n");
        goto out_free_output;
    }

    ret = EXIT_SUCCESS;
out_free_output:
    free(output);
out_free_base64:
    free(base64);
    return ret;
}
