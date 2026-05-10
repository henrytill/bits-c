#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits.h"
#include "printf.h"

extern size_t const expectedlen;
extern Message const expected[];

static unsigned const delay = 1U;

static uint8_t const cap = 1U;

static void *produce(void *data)
{
    size_t i = 0;
    int rc = 1;
    Channel *c = data;
    assert(data != NULL);

    for (i = 0; i < expectedlen; ++i) {
        for (rc = 1; rc == 1;) {
            rc = channelput(c, (Message *)&expected[i]);
            if (rc < 0) {
                eprintf("channelput failed: error %d", rc);
                exit(EXIT_FAILURE);
            }
        }
    }

    return NULL;
}

static int consume(Channel *c)
{
    size_t i = 0;
    Message *m;

    m = malloc(expectedlen * sizeof(Message));
    if (m == NULL)
        return 1;

    for (i = 0; i < expectedlen; ++i) {
        sleep(delay);
        channelget(c, &m[i]);
    }

    for (i = 0; i < expectedlen; ++i) {
        if (m[i].tag != expected[i].tag || m[i].value != expected[i].value) {
            free(m);
            return 1;
        }
    }

    free(m);
    return 0;
}

int main(void)
{
    int rc;
    int ret = EXIT_FAILURE;
    Channel *c;
    pthread_attr_t tattr;
    pthread_t tid;
    void *tret = NULL;

    c = channelcreate(cap);
    if (c == NULL) {
        eprintf("channelcreate failed");
        exit(EXIT_FAILURE);
    }

    rc = pthread_attr_init(&tattr);
    if (rc != 0) {
        errno = rc;
        perror("pthread_attr_init");
        goto destroyc;
    }

    rc = pthread_create(&tid, &tattr, &produce, c);
    if (rc != 0) {
        errno = rc;
        perror("pthread_create");
        goto destroytattr;
    }

    rc = consume(c);
    if (rc != 0) {
        eprintf("consume failed");
        goto destroytattr;
    }

    rc = pthread_join(tid, &tret);
    if (rc != 0) {
        errno = rc;
        perror("pthread_join");
        goto destroytattr;
    }

    assert(tret == NULL);

    ret = EXIT_SUCCESS;
destroytattr:
    (void)pthread_attr_destroy(&tattr);
destroyc:
    channeldestroy(c);
    return ret;
}
