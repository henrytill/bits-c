#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

static size_t base64_length(const char *in) {
  return ((strlen(in) + 2) / 3) * 4;
}

static long base64_encode(const unsigned char *in, int in_len, unsigned char *out) {
  return EVP_EncodeBlock(out, in, in_len);
}

static long base64_decode(const unsigned char *in, int in_len, unsigned char *out) {
  return EVP_DecodeBlock(out, in, in_len);
}

int main(void) {
  const char *input = "Hello, World!\n";

  const size_t base64_len = base64_length(input);
  char *base64 = malloc(base64_len + 1);

  long len = base64_encode((const unsigned char *)input,
                           (int)strlen(input),
                           (unsigned char *)base64);

  assert(len == (long)base64_len);

  printf("base64_len: %ld\n", base64_len);
  printf("base64: %s\n", base64);

  const size_t output_len = strlen(input) + 1;
  char *output = malloc(output_len);

  len = base64_decode((const unsigned char *)base64,
                      (int)base64_len,
                      (unsigned char *)output);

  assert(len == (long)output_len);

  printf("output_len: %ld\n", output_len);
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
