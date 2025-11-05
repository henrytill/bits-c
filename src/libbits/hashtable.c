#include <assert.h>
#include <stdlib.h>

#include "bits.h"
#include "macro.h"
#include "printf.h"

typedef struct Entry Entry;

struct Entry
{
    Entry *next;
    char const *key;
    void *value;
    int deleted;
};

struct Table
{
    size_t len;
    Entry columns[1]; /* C89 flexible array member workaround */
};

Table *tablecreate(size_t const len)
{
    Table *ret;

    if (!ISPOW2(len))
    {
        eprintf("len must be a power of 2\n");
        return NULL;
    }

    ret = calloc(1, sizeof(*ret) + ((len - 1) * sizeof(Entry)));
    if (ret == NULL)
        return NULL;

    ret->len = len;
    return ret;
}

void tabledestroy(Table *t, void finalize(void *))
{
    size_t i;
    Entry *curr, *next;

    if (t == NULL)
        return;

    for (i = 0; i < t->len; ++i)
    {
        curr = t->columns[i].next;
        while (curr != NULL)
        {
            next = curr->next;
            if (finalize != NULL && curr->value != NULL)
                finalize(curr->value);

            free((char *)curr->key);
            free(curr);
            curr = next;
        }

        if (finalize != NULL && t->columns[i].value != NULL)
            finalize(t->columns[i].value);
        if (t->columns[i].key != NULL)
            free((char *)t->columns[i].key);
    }
    free(t);
}

static uint64_t getindex(size_t const len, char const *key)
{
    uint64_t hash;

    assert(ISPOW2(len));
    assert(key != NULL);
    hash = fnv(strlen(key) + 1, (unsigned char const *)key);
    return hash & (uint64_t)(len - 1);
}

int tableput(Table *t, char const *key, void *value)
{
    uint64_t i;
    Entry *curr, *prev;

    if (t == NULL)
        return -1;

    if (key == NULL || value == NULL)
        return -1;

    i = getindex(t->len, key);
    debugprintf("key: %s index: %d\n", key, i);
    curr = &t->columns[i];

    while (curr != NULL && curr->key != NULL && !curr->deleted && strcmp(key, curr->key) != 0)
    {
        prev = curr;
        curr = curr->next;
    }

    /* existing active node */
    if (curr != NULL && curr->key != NULL && !curr->deleted)
    {
        curr->value = value;
        return 0;
    }

    /* uninitialized or deleted node - reuse it */
    if (curr != NULL)
    {
        if (curr->deleted && curr->key != NULL)
            free((char *)curr->key);
        curr->key = strdup(key);
        if (curr->key == NULL)
            return -1;
        curr->value = value;
        curr->deleted = 0;
        return 0;
    }

    /* new node */
    curr = calloc(1, sizeof(*curr));
    if (curr == NULL)
        return -1;

    curr->next = NULL;
    curr->key = strdup(key);
    if (curr->key == NULL)
    {
        free(curr);
        return -1;
    }

    curr->value = value;
    assert(prev != NULL);
    prev->next = curr;

    return 0;
}

void *tableget(Table *t, char const *key)
{
    uint64_t i;
    Entry *curr;

    if (t == NULL)
        return NULL;

    if (key == NULL)
        return NULL;

    i = getindex(t->len, key);
    debugprintf("key: %s index: %d\n", key, i);
    curr = &t->columns[i];

    while (curr != NULL && (curr->key == NULL || curr->deleted || strcmp(key, curr->key) != 0))
        curr = curr->next;

    if (curr == NULL)
        return NULL;

    return curr->value;
}

int tabledel(Table *t, char const *key, void finalize(void *))
{
    uint64_t i;
    Entry *curr;

    if (t == NULL)
        return -1;

    if (key == NULL)
        return -1;

    i = getindex(t->len, key);
    curr = &t->columns[i];

    while (curr != NULL && (curr->key == NULL || curr->deleted || strcmp(key, curr->key) != 0))
        curr = curr->next;

    /* not found */
    if (curr == NULL)
        return -1;

    /* found - mark as deleted */
    if (curr->value != NULL && finalize != NULL)
        finalize(curr->value);

    free((char *)curr->key);
    curr->key = NULL;
    curr->value = NULL;
    curr->deleted = 1;

    return 0;
}

void tablecompact(Table *t)
{
    size_t i;
    Entry *curr, *prev, *next, *replacement;

    if (t == NULL)
        return;

    for (i = 0; i < t->len; ++i)
    {
        curr = &t->columns[i];

        /* handle embedded entry */
        if (curr->deleted && curr->next != NULL)
        {
            /* move first non-deleted chained entry to embedded slot */
            replacement = curr->next;
            while (replacement != NULL && replacement->deleted)
                replacement = replacement->next;

            if (replacement != NULL)
            {
                /* copy replacement data to embedded entry */
                curr->key = replacement->key;
                curr->value = replacement->value;
                curr->deleted = 0;
                /* mark replacement as deleted for chain cleanup */
                replacement->deleted = 1;
            }
        }

        /* clean up deleted entries in chain */
        prev = curr;
        curr = curr->next;
        while (curr != NULL)
        {
            next = curr->next;
            if (curr->deleted)
            {
                prev->next = next;
                free(curr);
            }
            else
            {
                prev = curr;
            }
            curr = next;
        }
    }
}
