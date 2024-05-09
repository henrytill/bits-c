#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

static const struct test_vector {
    const char *key;
    char *value;
} TEST_VECTORS[] = {
#define X(prefix) {#prefix "_key", #prefix "_value"},
#include "hashtable_vectors.def"
#undef X
    {NULL, NULL},
};

int main(void)
{
    extern const struct test_vector TEST_VECTORS[];

    int ret = EXIT_FAILURE;
    const char *key = NULL;
    char *value = NULL;
    struct table *t = table_create(8);

    value = table_get(t, "not_in_table");
    if (value != NULL) {
        goto out_table_destroy;
    }

    for (size_t i = 0; (key = TEST_VECTORS[i].key) != NULL; ++i) {
        table_put(t, key, TEST_VECTORS[i].value);
    }

    for (size_t i = 0; (key = TEST_VECTORS[i].key) != NULL; ++i) {
        value = table_get(t, key);
        if (strcmp(TEST_VECTORS[i].value, value) != 0) {
            goto out_table_destroy;
        }
    }

    value = table_get(t, "not_in_table");
    if (value != NULL) {
        goto out_table_destroy;
    }

    ret = EXIT_SUCCESS;
out_table_destroy:
    table_destroy(t, NULL);
    return ret;
}
