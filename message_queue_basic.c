/// Test basic message queue functionality.
///
/// The producer thread produces messages with values from 0 to COUNT.
/// The consumer consumes messages on the main thread until it receives a
/// message with tag MSG_TAG_QUIT.
///
/// @see message_queue_create()
/// @see message_queue_put()
/// @see message_queue_get()
/// @see message_queue_destroy()
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "message_queue.h"

/// Maximum value to Produce.
static const int COUNT = 100;

/// Capacity of the message_queue.
static const uint32_t QUEUE_CAP = 4U;

/// Logs an error message and exit.
static void fail(const char *msg) {
  (void)fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/// Logs a message_queue error message and exit.
static void message_queue_fail(int rc, const char *msg) {
  (void)fprintf(stderr, "%s: %s\n", msg, message_queue_failure_str((enum message_queue_failure)(-rc)));
  exit(EXIT_FAILURE);
}

/// Produces messages with values from 0 to COUNT. The last message has tag MSG_TAG_QUIT.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a message_queue.
/// @return 0 on success
/// @see consume()
static void *produce(void *data) {
  extern const int COUNT;

  if (data == NULL) {
    fail("produce failed: data is NULL");
  }

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
      printf("produce {%s, %" PRIdPTR "} blocked: retrying\n", tag_str, value);
    } else {
      printf("Produced {%s, %" PRIdPTR "}\n", tag_str, value);
      value += 1;
    }
  }

  return NULL;
}

/// Consumes messages until a message with tag MSG_TAG_QUIT is received.
///
/// This function is meant to be run on the main thread.
///
/// @param queue Pointer to a message_queue.
/// @param out Pointer to a message.
/// @return 0 when a message with tag MSG_TAG_QUIT is received, 1 otherwise
/// @see produce()
static int consume(struct message_queue *queue, struct message *out) {
  const int rc = message_queue_get(queue, out);
  if (rc < 0) {
    message_queue_fail(rc, "message_queue_get failed");
  }
  printf("Consumed {%s, %" PRIdPTR "}\n", message_tag_str(out->tag), out->value);
  return out->tag != MSG_TAG_QUIT;
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  extern const uint32_t QUEUE_CAP;

  int ret = EXIT_FAILURE;

  struct message_queue *queue = message_queue_create(QUEUE_CAP);
  if (queue == NULL) {
    fail("message_queue_create failed");
  }

  int rc = -1;
  pthread_attr_t attr;
  pthread_t thread_id;

  rc = pthread_attr_init(&attr);
  if (rc != 0) {
    errno = rc;
    perror("pthread_attr_init");
    goto out_destroy_queue;
  }

  rc = pthread_create(&thread_id, &attr, &produce, queue);
  if (rc != 0) {
    errno = rc;
    perror("pthread_attr_init");
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
  rc = pthread_join(thread_id, &thread_ret);
  if (rc != 0) {
    errno = rc;
    perror("pthread_attr_init");
    goto out_destroy_attr;
  }

  assert(thread_ret == NULL);

  ret = EXIT_SUCCESS;
out_destroy_attr:
  (void)pthread_attr_destroy(&attr);
out_destroy_queue:
  message_queue_destroy(queue);
  return ret;
}
