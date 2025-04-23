#include "message_queue.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>

struct message_queue {
  /// Buffer to hold messages
  struct message *buffer;
  /// Maximum size of the buffer
  uint32_t capacity;
  /// Index of the front message in the buffer
  size_t front;
  /// Index of the rear message in the buffer
  size_t rear;
  /// Semaphore to track empty slots in the buffer
  sem_t *empty;
  /// Semaphore to track filled slots in the buffer
  sem_t *full;
  /// Mutex lock to protect buffer access
  pthread_mutex_t *lock;
};

/// Create a semaphore.
///
/// @param value The initial value of the semaphore.
/// @return The semaphore, or NULL on failure.
static sem_t *create_semaphore(uint32_t value) {
  sem_t *sem = calloc(1, sizeof(*sem));
  if (sem == NULL) {
    return NULL;
  }

  int const rc = sem_init(sem, 0, value);
  if (rc == -1) {
    free(sem);
    return NULL;
  }

  return sem;
}

/// Destroy a semaphore.
///
/// @param sem The semaphore to destroy.
static void destroy_semaphore(sem_t *sem) {
  if (sem == NULL) {
    return;
  }

  sem_destroy(sem);
  free(sem);
}

/// Create a mutex.
///
/// @return The mutex, or NULL on failure.
static pthread_mutex_t *create_mutex(void) {
  pthread_mutex_t *mutex = calloc(1, sizeof(pthread_mutex_t));
  if (mutex == NULL) {
    return NULL;
  }

  int const rc = pthread_mutex_init(mutex, NULL);
  if (rc != 0) {
    free(mutex);
    return NULL;
  }

  return mutex;
}

/// Destroy a mutex.
///
/// @param mutex The mutex to destroy.
static void destroy_mutex(pthread_mutex_t *mutex) {
  if (mutex == NULL) {
    return;
  }

  pthread_mutex_destroy(mutex);
  free(mutex);
}

static int message_queue_init(struct message_queue *queue, uint32_t capacity) {
  if (queue == NULL) {
    return -MSGQ_FAILURE_NULL_POINTER;
  }

  queue->buffer = calloc((size_t)capacity, sizeof(*queue->buffer));
  if (queue->buffer == NULL) {
    return -MSGQ_FAILURE_MALLOC;
  }

  queue->capacity = capacity;
  queue->front = 0;
  queue->rear = 0;

  queue->empty = create_semaphore(capacity);
  if (queue->empty == NULL) {
    free(queue->buffer);
    return -MSGQ_FAILURE_SEM_CREATE;
  }

  queue->full = create_semaphore(0);
  if (queue->full == NULL) {
    destroy_semaphore(queue->empty);
    free(queue->buffer);
    return -MSGQ_FAILURE_SEM_CREATE;
  }

  queue->lock = create_mutex();
  if (queue->lock == NULL) {
    destroy_semaphore(queue->full);
    destroy_semaphore(queue->empty);
    free(queue->buffer);
    return -MSGQ_FAILURE_MUTEX_CREATE;
  }

  return 0;
}

static void message_queue_finish(struct message_queue *queue) {
  if (queue == NULL) {
    return;
  }

  queue->capacity = 0;
  queue->front = 0;
  queue->rear = 0;

  if (queue->buffer != NULL) {
    free(queue->buffer);
    queue->buffer = NULL;
  }
  if (queue->empty != NULL) {
    destroy_semaphore(queue->empty);
    queue->empty = NULL;
  }
  if (queue->full != NULL) {
    destroy_semaphore(queue->full);
    queue->full = NULL;
  }
  if (queue->lock != NULL) {
    destroy_mutex(queue->lock);
    queue->lock = NULL;
  }
}

struct message_queue *message_queue_create(uint32_t capacity) {
  struct message_queue *queue = calloc(1, sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }

  int rc = message_queue_init(queue, capacity);
  if (rc < 0) {
    free(queue);
    return NULL;
  }

  return queue;
}

void message_queue_destroy(struct message_queue *queue) {
  if (queue == NULL) {
    return;
  }

  message_queue_finish(queue);
  free(queue);
}

int message_queue_put(struct message_queue *queue, struct message *in) {
  int rc = sem_trywait(queue->empty);
  if (rc == -1 && errno == EAGAIN) {
    return 1;
  }
  if (rc == -1) {
    return -MSGQ_FAILURE_SEM_TRY_WAIT;
  }

  rc = pthread_mutex_lock(queue->lock);
  if (rc != 0) {
    return -MSGQ_FAILURE_MUTEX_LOCK;
  }

  queue->buffer[queue->rear] = *in;
  queue->rear = (queue->rear + 1) % queue->capacity;

  rc = pthread_mutex_unlock(queue->lock);
  if (rc != 0) {
    return -MSGQ_FAILURE_MUTEX_UNLOCK;
  }

  rc = sem_post(queue->full);
  if (rc == -1) {
    return -MSGQ_FAILURE_SEM_POST;
  }

  return 0;
}

int message_queue_get(struct message_queue *queue, struct message *out) {
  int rc = sem_wait(queue->full);
  if (rc == -1) {
    return -MSGQ_FAILURE_SEM_WAIT;
  }

  rc = pthread_mutex_lock(queue->lock);
  if (rc != 0) {
    return -MSGQ_FAILURE_MUTEX_LOCK;
  }

  *out = queue->buffer[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;

  rc = pthread_mutex_unlock(queue->lock);
  if (rc != 0) {
    return -MSGQ_FAILURE_MUTEX_UNLOCK;
  }

  rc = sem_post(queue->empty);
  if (rc == -1) {
    return -MSGQ_FAILURE_SEM_POST;
  }

  return 0;
}

int message_queue_size(struct message_queue *queue) {
  if (queue == NULL) {
    return 0;
  }

  int ret = 0;
  sem_getvalue(queue->full, &ret);
  return ret;
}
