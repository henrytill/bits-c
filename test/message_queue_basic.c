#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "macro.h"
#include "message_queue.h"

static const int COUNT = 100;

static const uint32_t QUEUE_CAP = 4U;

static void fail(const char *msg)
{
    eprintf("%s\n", msg);
    exit(EXIT_FAILURE);
}

static void message_queue_fail(int rc, const char *msg)
{
    eprintf("%s: %s\n", msg, message_queue_failure_str((enum message_queue_failure)(-rc)));
    exit(EXIT_FAILURE);
}

static void *produce(void *data)
{
    extern const int COUNT;

    assert(data != NULL);

    struct message_queue *queue = data;
    struct message msg = {0};
    const char *tag_str = NULL;
    int rc = -1;

    for (intptr_t value = 0; value <= COUNT;) {
        msg.tag = (value < COUNT) ? MSG_TAG_SOME : MSG_TAG_QUIT;
        msg.value = value;
        tag_str = message_tag_str(msg.tag);

        rc = message_queue_put(queue, &msg);
        if (rc < 0) {
            message_queue_fail(rc, "message_queue_put failed");
        } else if (rc == 1) {
            printf("blocked: {%s, %" PRIdPTR "}\n", tag_str, value);
        } else {
            printf("produced: {%s, %" PRIdPTR "}\n", tag_str, value);
            value += 1;
        }
    }

    return NULL;
}

static int consume(struct message_queue *queue, struct message *out)
{
    const int rc = message_queue_get(queue, out);
    if (rc < 0) {
        message_queue_fail(rc, "message_queue_get failed");
    }
    printf("consumed: {%s, %" PRIdPTR "}\n", message_tag_str(out->tag), out->value);
    return out->tag != MSG_TAG_QUIT;
}

int main(void)
{
    extern const uint32_t QUEUE_CAP;

    int ret = EXIT_FAILURE;

    struct message_queue *queue = message_queue_create(QUEUE_CAP);
    if (queue == NULL) {
        fail("message_queue_create failed");
    }

    int rc = -1;
    pthread_attr_t thread_attr;
    pthread_t thread_id;

    rc = pthread_attr_init(&thread_attr);
    if (rc != 0) {
        errno = rc;
        perror("pthread_attr_init");
        goto out_destroy_queue;
    }

    rc = pthread_create(&thread_id, &thread_attr, produce, queue);
    if (rc != 0) {
        errno = rc;
        perror("pthread_create");
        goto out_destroy_attr;
    }

    struct message msg;
    for (;;) {
        rc = consume(queue, &msg);
        if (rc == 0) {
            break;
        }
        if (rc < 0) {
            goto out_destroy_queue;
        }
    }

    void *thread_ret = NULL;
    rc = pthread_join(thread_id, thread_ret);
    if (rc != 0) {
        errno = rc;
        perror("pthread_join");
        goto out_destroy_attr;
    }

    assert(thread_ret == NULL);

    ret = EXIT_SUCCESS;
out_destroy_attr:
    (void)pthread_attr_destroy(&thread_attr);
out_destroy_queue:
    message_queue_destroy(queue);
    return ret;
}
