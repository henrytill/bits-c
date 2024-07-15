/// Test that values are copied into and out of the message queue.
///
/// The producer thread produces messages with values 42, 0, and 1.
/// The consumer consumes messages on the main thread after a delay and checks
/// their values.
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
#include <unistd.h>

#include "message_queue.h"

#define LOG(_msg) ({                               \
  struct message __msg = (_msg);                   \
  printf("%s: %s{%s, %" PRIdPTR "}\n",             \
         __func__, #_msg,                          \
         message_tag_str(__msg.tag), __msg.value); \
})

#define CHECK(_msg, _tag, _value) ({                                 \
  struct message __msg = (_msg);                                     \
  enum message_tag __tag = (_tag);                                   \
  typeof(_value) __value = (_value);                                 \
  if (__msg.tag != __tag || __msg.value != __value) {                \
    (void)fprintf(stderr, "%s: %s{%s, %" PRIdPTR "} != {%s, %ld}\n", \
                  __func__, #_msg,                                   \
                  message_tag_str(__msg.tag), __msg.value,           \
                  message_tag_str(__tag), __value);                  \
    exit(EXIT_FAILURE);                                              \
  }                                                                  \
});

/// Delay before consuming messages.
static const unsigned DELAY = 2U;

/// Capacity of the message_queue.
static const uint32_t QUEUE_CAP = 1U;

/// Logs an error message and exit.
static void fail(const char *msg) {
  (void)fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

/// Logs a message_queue error message and exit.
static void message_queue_fail(int rc, const char *msg) {
  (void)fprintf(stderr, "%s: %s", msg, message_queue_failure_str((enum message_queue_failure)(-rc)));
  exit(EXIT_FAILURE);
}

/// Produces messages.
///
/// This function is meant to be run in its own thread by passing it to SDL_CreateThread().
///
/// @param data Pointer to a message_queue.
/// @return 0 on success, 1 on failure.
/// @see consume()
static void *produce(void *data) {
  struct message_queue *queue = data;
  struct message msg = {.tag = MSG_TAG_SOME, .value = 42};

  for (int rc = 1; rc == 1;) {
    rc = message_queue_put(queue, &msg);
    if (rc < 0) {
      message_queue_fail(rc, "message_queue_put failed");
    }
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 0;
  for (int rc = 1; rc == 1;) {
    rc = message_queue_put(queue, &msg);
    if (rc < 0) {
      message_queue_fail(rc, "message_queue_put failed");
    }
  }
  LOG(msg);

  msg.tag = MSG_TAG_SOME;
  msg.value = 1;
  for (int rc = 1; rc == 1;) {
    rc = message_queue_put(queue, &msg);
    if (rc < 0) {
      message_queue_fail(rc, "message_queue_put failed");
    }
  }
  LOG(msg);

  return NULL;
}

/// Consumes messages produced by produce() after a delay and checks their values.
///
/// This function is meant to be run in the main thread.
///
/// @param queue Pointer to a message_queue.
/// @return 0 on success, 1 on failure.
/// @see produce()
static int consume(struct message_queue *queue) {
  extern const unsigned DELAY;

  struct message a = {0};
  struct message b = {0};
  struct message c = {0};

  printf("%s: pausing for %d...\n", __func__, DELAY);
  sleep(DELAY);

  message_queue_get(queue, &a);
  LOG(a);
  CHECK(a, MSG_TAG_SOME, 42L);

  message_queue_get(queue, &b);
  LOG(b);
  CHECK(a, MSG_TAG_SOME, 42L);
  CHECK(b, MSG_TAG_SOME, 0L);

  message_queue_get(queue, &c);
  LOG(c);
  CHECK(a, MSG_TAG_SOME, 42L);
  CHECK(b, MSG_TAG_SOME, 0L);
  CHECK(c, MSG_TAG_SOME, 1L);

  return 0;
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

  if (consume(queue) != 0) {
    (void)fprintf(stderr, "consume failed");
    goto out_destroy_queue;
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
