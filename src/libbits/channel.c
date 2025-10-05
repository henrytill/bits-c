#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "bits.h"

struct Channel {
  Message *buffer;       /**< Buffer to hold messages */
  uint8_t capacity;      /**< Maximum size of the buffer */
  size_t front;          /**< Index of the front message in the buffer */
  size_t rear;           /**< Index of the rear message in the buffer */
  sem_t *empty;          /**< Semaphore to track empty slots in the buffer */
  sem_t *full;           /**< Semaphore to track filled slots in the buffer */
  pthread_mutex_t *lock; /**< Mutex lock to protect buffer access */
};

static sem_t *semcreate(uint32_t value) {
  int rc;
  sem_t *sem;

  sem = calloc(1, sizeof(*sem));
  if (sem == NULL)
    return NULL;

  rc = sem_init(sem, 0, value);
  if (rc == -1) {
    free(sem);
    return NULL;
  }

  return sem;
}

static void semdestroy(sem_t *sem) {
  if (sem == NULL)
    return;

  sem_destroy(sem);
  free(sem);
}

static pthread_mutex_t *mutexcreate(void) {
  int rc;
  pthread_mutex_t *mutex;

  mutex = calloc(1, sizeof(pthread_mutex_t));
  if (mutex == NULL)
    return NULL;

  rc = pthread_mutex_init(mutex, NULL);
  if (rc != 0) {
    free(mutex);
    return NULL;
  }

  return mutex;
}

static void mutexdestroy(pthread_mutex_t *mutex) {
  if (mutex == NULL)
    return;

  pthread_mutex_destroy(mutex);
  free(mutex);
}

static int channelinit(Channel *c, uint8_t capacity) {
  if (c == NULL || capacity == 0)
    return -1;

  c->buffer = calloc((size_t)capacity, sizeof(*c->buffer));
  if (c->buffer == NULL)
    return -1;

  c->capacity = capacity;
  c->front = 0;
  c->rear = 0;

  c->empty = semcreate(capacity);
  if (c->empty == NULL)
    goto freebuffer;

  c->full = semcreate(0);
  if (c->full == NULL)
    goto destroyempty;

  c->lock = mutexcreate();
  if (c->lock == NULL)
    goto destroyfull;

  return 0;

destroyfull:
  semdestroy(c->full);
destroyempty:
  semdestroy(c->empty);
freebuffer:
  free(c->buffer);
  return -1;
}

static void channelfinish(Channel *c) {
  if (c == NULL)
    return;

  c->capacity = 0;
  c->front = 0;
  c->rear = 0;

  if (c->buffer != NULL) {
    free(c->buffer);
    c->buffer = NULL;
  }
  if (c->empty != NULL) {
    semdestroy(c->empty);
    c->empty = NULL;
  }
  if (c->full != NULL) {
    semdestroy(c->full);
    c->full = NULL;
  }
  if (c->lock != NULL) {
    mutexdestroy(c->lock);
    c->lock = NULL;
  }
}

Channel *channelcreate(uint8_t capacity) {
  int rc;
  Channel *c;

  c = calloc(1, sizeof(*c));
  if (c == NULL)
    return NULL;

  rc = channelinit(c, capacity);
  if (rc != 0) {
    free(c);
    return NULL;
  }

  return c;
}

void channeldestroy(Channel *c) {
  int f, e;

  if (c == NULL)
    return;

  do {
    sem_getvalue(c->empty, &e);
    sem_getvalue(c->full, &f);

    if (f > 0) {
      sem_wait(c->full);
      sem_post(c->full);
    }
  } while (f > 0 || e != c->capacity);

  channelfinish(c);
  free(c);
}

int channelput(Channel *c, struct Message *in) {
  int rc;

  if (c == NULL || in == NULL)
    return -1;

  rc = sem_trywait(c->empty);
  if (rc == -1)
    return (errno == EAGAIN) ? 1 : -1;

  rc = pthread_mutex_lock(c->lock);
  if (rc != 0) {
    sem_post(c->empty);
    return -1;
  }

  c->buffer[c->rear] = *in;
  c->rear = (c->rear + 1) % c->capacity;

  rc = pthread_mutex_unlock(c->lock);
  if (rc != 0) {
    sem_post(c->empty);
    return -1;
  }

  rc = sem_post(c->full);
  if (rc == -1) {
    sem_post(c->empty);
    return -1;
  }

  return 0;
}

int channelget(Channel *c, struct Message *out) {
  int rc;

  if (c == NULL || out == NULL)
    return -1;

  rc = sem_wait(c->full);
  if (rc == -1)
    return -1;

  rc = pthread_mutex_lock(c->lock);
  if (rc != 0) {
    sem_post(c->full);
    return -1;
  }

  *out = c->buffer[c->front];
  c->front = (c->front + 1) % c->capacity;

  rc = pthread_mutex_unlock(c->lock);
  if (rc != 0) {
    sem_post(c->full);
    return -1;
  }

  rc = sem_post(c->empty);
  if (rc == -1) {
    sem_post(c->full);
    return -1;
  }

  return 0;
}

int channelsize(Channel *c) {
  int rc, ret;

  if (c == NULL)
    return 0;

  rc = sem_getvalue(c->full, &ret);
  if (rc == -1)
    return -1;

  return ret;
}
