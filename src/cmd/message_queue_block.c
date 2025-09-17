#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits.h"

extern size_t const EXPECTED_LEN;
extern struct message const EXPECTED[];

static unsigned const DELAY = 1U;

static uint32_t const QUEUE_CAP = 1U;

static void *
produce(void *data)
{
	struct message_queue *queue = data;
	size_t i = 0;
	int rc = 1;
	assert(data != NULL);

	for(i = 0; i < EXPECTED_LEN; ++i) {
		for(rc = 1; rc == 1;) {
			rc = message_queue_put(queue, (struct message *)&EXPECTED[i]);
			if(rc < 0) {
				eprintf("%s", message_queue_failure_str((enum message_queue_failure)(-rc)));
				exit(EXIT_FAILURE);
			}
		}
	}

	return NULL;
}

static int
consume(struct message_queue *queue)
{
	size_t i = 0;
	struct message *consumed;

	consumed = malloc(EXPECTED_LEN * sizeof(struct message));
	if(consumed == NULL) {
		return 1;
	}

	for(i = 0; i < EXPECTED_LEN; ++i) {
		sleep(DELAY);
		message_queue_get(queue, &consumed[i]);
	}

	for(i = 0; i < EXPECTED_LEN; ++i) {
		if(consumed[i].tag != EXPECTED[i].tag || consumed[i].value != EXPECTED[i].value) {
			free(consumed);
			return 1;
		}
	}

	free(consumed);
	return 0;
}

int
main(void)
{
	int ret = EXIT_FAILURE;
	struct message_queue *queue;
	pthread_attr_t thread_attr;
	int rc;
	pthread_t thread_id;
	void *thread_ret = NULL;

	queue = message_queue_create(QUEUE_CAP);
	if(queue == NULL) {
		eprintf("message_queue_create failed");
		exit(EXIT_FAILURE);
	}

	rc = pthread_attr_init(&thread_attr);
	if(rc != 0) {
		errno = rc;
		perror("pthread_attr_init");
		goto out_destroy_queue;
	}

	rc = pthread_create(&thread_id, &thread_attr, &produce, queue);
	if(rc != 0) {
		errno = rc;
		perror("pthread_create");
		goto out_destroy_attr;
	}

	rc = consume(queue);
	if(rc != 0) {
		eprintf("consume failed");
		goto out_destroy_attr;
	}

	rc = pthread_join(thread_id, &thread_ret);
	if(rc != 0) {
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
