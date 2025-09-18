#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits.h"

extern size_t const expectedlen;
extern Message const expected[];

static unsigned const delay = 1U;

static uint32_t const channelcap = 1U;

static void *
produce(void *data)
{
	size_t i = 0;
	int rc = 1;
	Channel *c = data;
	assert(data != NULL);

	for(i = 0; i < expectedlen; ++i) {
		for(rc = 1; rc == 1;) {
			rc = channelput(c, (Message *)&expected[i]);
			if(rc < 0) {
				eprintf("channelput failed: error %d", rc);
				exit(EXIT_FAILURE);
			}
		}
	}

	return NULL;
}

static int
consume(Channel *c)
{
	size_t i = 0;
	Message *m;

	m = malloc(expectedlen * sizeof(Message));
	if(m == NULL)
		return 1;

	for(i = 0; i < expectedlen; ++i) {
		sleep(delay);
		channelget(c, &m[i]);
	}

	for(i = 0; i < expectedlen; ++i) {
		if(m[i].tag != expected[i].tag || m[i].value != expected[i].value) {
			free(m);
			return 1;
		}
	}

	free(m);
	return 0;
}

int
main(void)
{
	int rc;
	int ret = EXIT_FAILURE;
	Channel *c;
	pthread_attr_t tattr;
	pthread_t tid;
	void *tret = NULL;

	c = channelcreate(channelcap);
	if(c == NULL) {
		eprintf("channelcreate failed");
		exit(EXIT_FAILURE);
	}

	rc = pthread_attr_init(&tattr);
	if(rc != 0) {
		errno = rc;
		perror("pthread_attr_init");
		goto out_channeldestroy_c;
	}

	rc = pthread_create(&tid, &tattr, &produce, c);
	if(rc != 0) {
		errno = rc;
		perror("pthread_create");
		goto out_destroy_attr;
	}

	rc = consume(c);
	if(rc != 0) {
		eprintf("consume failed");
		goto out_destroy_attr;
	}

	rc = pthread_join(tid, &tret);
	if(rc != 0) {
		errno = rc;
		perror("pthread_join");
		goto out_destroy_attr;
	}

	assert(tret == NULL);

	ret = EXIT_SUCCESS;
out_destroy_attr:
	(void)pthread_attr_destroy(&tattr);
out_channeldestroy_c:
	channeldestroy(c);
	return ret;
}
