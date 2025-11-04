#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>

#include "bits.h"
#include "printf.h"

static int const count = 100;

static uint8_t const cap = 4U;

static void fail(char const *msg)
{
    eprintf("%s\n", msg);
    exit(EXIT_FAILURE);
}

static void channelfail(int rc, char const *msg)
{
    eprintf("%s: error %d\n", msg, rc);
    exit(EXIT_FAILURE);
}

static void *produce(void *data)
{
    int rc = -1;
    Channel *c;
    Message m;
    intptr_t v;

    assert(data != NULL);

    c = data;

    for (v = 0; v <= count;) {
        m.tag = (v < count) ? Tsome : Tclose;
        m.value = v;

        rc = channelput(c, &m);
        if (rc < 0)
            channelfail(rc, "channelput failed");
        else if (rc == 1)
            printf("blocked: {%d, %" PRIdPTR "}\n", m.tag, v);
        else {
            printf("produced: {%d, %" PRIdPTR "}\n", m.tag, v);
            v += 1;
        }
    }

    return NULL;
}

static int consume(Channel *c, Message *out)
{
    int const rc = channelget(c, out);
    if (rc < 0)
        channelfail(rc, "channelget failed");

    printf("consumed: {%d, %" PRIdPTR "}\n", out->tag, out->value);
    return out->tag != Tclose;
}

int main(void)
{
    int rc;
    int ret = EXIT_FAILURE;
    Channel *c;
    Message m;
    pthread_attr_t tattr;
    pthread_t tid;
    void *tret = NULL;

    c = channelcreate(cap);
    if (c == NULL)
        fail("channelcreate failed");

    rc = pthread_attr_init(&tattr);
    if (rc != 0) {
        errno = rc;
        perror("pthread_attr_init");
        goto destroyc;
    }

    rc = pthread_create(&tid, &tattr, produce, c);
    if (rc != 0) {
        errno = rc;
        perror("pthread_create");
        goto destroytattr;
    }

    for (;;) {
        rc = consume(c, &m);
        if (rc == 0)
            break;

        if (rc < 0)
            goto destroyc;
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
