#pragma once

#include <stdint.h>
#include <stdio.h> /* IWYU pragma: keep */

#include "macro.h"

void eprintf(char const *fmt, ...);

void debug_printf(char const *fmt, ...);
void debug_eprintf(char const *fmt, ...);

void *arena_allocate(size_t n, size_t t);
void arena_deallocate(size_t t);
void arena_free(size_t t);

uint64_t fnv_hash(size_t data_len, unsigned char const *data);

struct table;

struct table *table_create(size_t columns_len);
void table_destroy(struct table *t, void finalize(void *));
int table_put(struct table *t, char const *key, void *value);
void *table_get(struct table *t, char const *key);
int table_delete(struct table *t, char const *key, void finalize(void *));

enum message_queue_failure {
	MSGQ_FAILURE_NULL_POINTER = 1,
	MSGQ_FAILURE_MALLOC = 2,
	MSGQ_FAILURE_SEM_CREATE = 3,
	MSGQ_FAILURE_SEM_POST = 4,
	MSGQ_FAILURE_SEM_TRY_WAIT = 5,
	MSGQ_FAILURE_SEM_WAIT = 6,
	MSGQ_FAILURE_MUTEX_CREATE = 7,
	MSGQ_FAILURE_MUTEX_LOCK = 8,
	MSGQ_FAILURE_MUTEX_UNLOCK = 9,
	MSGQ_FAILURE_MAX = 10
};

enum tag {
	MSG_TAG_NONE = 0,
	MSG_TAG_SOME = 1,
	MSG_TAG_QUIT = 2,
	MSG_TAG_MAX = 3
};

struct message {
	enum tag tag;
	intptr_t value;
};

struct message_queue;

struct message_queue *message_queue_create(uint32_t capacity);
void message_queue_destroy(struct message_queue *queue);
int message_queue_put(struct message_queue *queue, struct message *in);
int message_queue_get(struct message_queue *queue, struct message *out);
int message_queue_size(struct message_queue *queue);
char const *message_queue_failure_str(enum message_queue_failure failure);
char const *message_tag_str(enum tag tag);
