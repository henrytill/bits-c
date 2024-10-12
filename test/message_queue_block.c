#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "macro.h"
#include "message_queue.h"

static const unsigned DELAY = 1U;

static const uint32_t QUEUE_CAP = 1U;

static void *produce(void *data)
{
    extern const size_t EXPECTED_LEN;       // NOLINT(readability-identifier-naming)
    extern const struct message EXPECTED[]; // NOLINT(readability-identifier-naming)

    assert(data != NULL);
    struct message_queue *queue = data;

    for (size_t i = 0; i < EXPECTED_LEN; ++i) {
        for (int rc = 1; rc == 1;) {
            rc = message_queue_put(queue, (struct message *)&EXPECTED[i]);
            if (rc < 0) {
                eprintf("%s: %s", __func__, message_queue_failure_str((enum message_queue_failure)(-rc)));
                exit(EXIT_FAILURE);
            }
        }
    }

    return NULL;
}

static int consume(struct message_queue *queue)
{
    extern const unsigned DELAY;
    extern const size_t EXPECTED_LEN;       // NOLINT(readability-identifier-naming)
    extern const struct message EXPECTED[]; // NOLINT(readability-identifier-naming)

    struct message consumed[EXPECTED_LEN];

    for (size_t i = 0; i < EXPECTED_LEN; ++i) {
        sleep(DELAY);
        message_queue_get(queue, &consumed[i]);
    }

    for (size_t i = 0; i < EXPECTED_LEN; ++i) {
        if (consumed[i].tag != EXPECTED[i].tag || consumed[i].value != EXPECTED[i].value) {
            return 1;
        }
    }

    return 0;
}

int main(void)
{
    extern const uint32_t QUEUE_CAP;

    int ret = EXIT_FAILURE;

    struct message_queue *queue = message_queue_create(QUEUE_CAP);
    if (queue == NULL) {
        eprintf("message_queue_create failed");
        exit(EXIT_FAILURE);
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

    rc = pthread_create(&thread_id, &thread_attr, &produce, queue);
    if (rc != 0) {
        errno = rc;
        perror("pthread_create");
        goto out_destroy_attr;
    }

    rc = consume(queue);
    if (rc != 0) {
        eprintf("consume failed");
        goto out_destroy_attr;
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
