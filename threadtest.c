/* This allows us to use strdup */
#define _POSIX_C_SOURCE 200809L /* NOLINT */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct thread_info { /* Used as argument to thread_start() */
  pthread_t tid;             /* ID returned by pthread_create() */
  int tnum;                  /* Application-defined thread # */
  char *arg;                 /* From command-line argument */
} thread_info;

static inline void handle_errno(int err, const char *msg) {
  errno = err;
  perror(msg);
  exit(EXIT_FAILURE);
}

static inline void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Thread start function: display address near top of our stack,
   and return upper-cased copy of argv_string. */
static void *start(void *arg) {
  thread_info *tinfo = arg;
  printf("Thread %d: top of stack near %p; argv_string=%s\n",
         tinfo->tnum, (void *)&tinfo, tinfo->arg);
  char *ret = strdup(tinfo->arg);
  if (ret == NULL) {
    handle_error("strdup");
  }
  for (char *p = ret; *p != '\0'; ++p) {
    *p = (char)toupper(*p);
  }
  return ret;
}

int main(int argc, char *argv[]) {
  int rc;
  int opt;
  int nthreads;
  pthread_attr_t attr;
  size_t stacksz;
  void *res;

  /* The "-s" option specifies a stack size for our threads. */
  stacksz = 0;
  while ((opt = getopt(argc, argv, "s:")) != -1) {
    switch (opt) {
    case 's':
      stacksz = strtoul(optarg, NULL, 0);
      break;
    default:
      rc = fprintf(stderr, "Usage: %s [-s stack-size] arg...\n", argv[0]);
      assert(rc > 0);
      exit(EXIT_FAILURE);
    }
  }

  nthreads = argc - optind;

  /* Initialize thread creation attributes. */
  rc = pthread_attr_init(&attr);
  if (rc != 0) {
    handle_errno(rc, "pthread_attr_init");
  }

  if (stacksz > 0) {
    rc = pthread_attr_setstacksize(&attr, stacksz);
    if (rc != 0) {
      handle_errno(rc, "pthread_attr_setstacksize");
    }
  }

  /* Allocate memory for pthread_create() arguments. */
  thread_info *tinfo = calloc((size_t)nthreads, sizeof(*tinfo));
  if (tinfo == NULL) {
    handle_error("calloc");
  }

  /* Create one thread for each command-line argument. */
  for (int i = 0; i < nthreads; ++i) {
    tinfo[i].tnum = i + 1;
    tinfo[i].arg = argv[optind + i];

    /* The pthread_create() call stores the thread ID into corresponding element of tinfo[]. */
    rc = pthread_create(&tinfo[i].tid, &attr, &start, &tinfo[i]);
    if (rc != 0) {
      handle_errno(rc, "pthread_create");
    }
  }

  /* Destroy the thread attributes object, since it is no longer needed. */
  rc = pthread_attr_destroy(&attr);
  if (rc != 0) {
    handle_errno(rc, "pthread_attr_destroy");
  }

  /* Now join with each thread, and display its returned value. */
  for (int i = 0; i < nthreads; ++i) {
    rc = pthread_join(tinfo[i].tid, &res);
    if (rc != 0) {
      handle_errno(rc, "pthread_join");
    }
    printf("Joined with thread %d; returned value was %s\n", tinfo[i].tnum, (char *)res);
    free(res); /* Free memory allocated by thread */
  }

  free(tinfo);
  return EXIT_SUCCESS;
}
