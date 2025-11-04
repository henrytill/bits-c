#include <stdlib.h>
#include <string.h>

#include "bits.h"

static struct
{
    char const *key;
    char *value;
} const vectors[] = {
#define X(prefix) { #prefix "_key", #prefix "_value" },
#include "hashtable_vectors.def"
#undef X
    { NULL, NULL },
};

int main(void)
{
    int ret = EXIT_FAILURE;
    char const *key = NULL;
    char *value = NULL;
    struct Table *t;
    int rc = -1;
    size_t i;

    t = tablecreate(8);

    value = tableget(t, "not_in_table");
    if (value != NULL)
        goto destroyt;

    for (i = 0; (key = vectors[i].key) != NULL; ++i)
        tableput(t, key, vectors[i].value);

    for (i = 0; (key = vectors[i].key) != NULL; ++i)
    {
        value = tableget(t, key);
        if (strcmp(vectors[i].value, value) != 0)
            goto destroyt;
    }

    value = tableget(t, "not_in_table");
    if (value != NULL)
        goto destroyt;

    for (i = 0; (key = vectors[i].key) != NULL; ++i)
    {
        rc = tabledel(t, key, NULL);
        if (rc != 0)
            goto destroyt;
    }

    for (i = 0; (key = vectors[i].key) != NULL; ++i)
    {
        value = tableget(t, key);
        if (value != NULL)
            goto destroyt;
    }

    ret = EXIT_SUCCESS;
destroyt:
    tabledestroy(t, NULL);
    return ret;
}
