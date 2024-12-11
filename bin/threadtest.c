#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macro.h"

#define MAX_THREADS 128

struct thread_info {
  pthread_t thread_id;
  int thread_num;
  char *arg;
};

static struct thread_info info[MAX_THREADS];

static void handle_errno(int err, const char *msg) {
  errno = err;
  perror(msg);
  exit(EXIT_FAILURE);
}

static void *start(void *data) {
  struct thread_info *info = data;
  printf("Thread %d: top of stack near %p; argv_string=%s\n",
         info->thread_num, (void *)&info, info->arg);
  for (char *p = info->arg; *p != '\0'; ++p) {
    *p = (char)toupper(*p);
  }
  return info->arg;
}

static void run_threads(size_t stack_size, int num_threads, char *args[]) {
  printf("num_threads: %d\n", num_threads);
  if (num_threads < 1) {
    return;
  }

  int rc = -1;
  pthread_attr_t attr;
  rc = pthread_attr_init(&attr);
  if (rc != 0) {
    handle_errno(rc, "pthread_attr_init");
  }

  if (stack_size > 0) {
    rc = pthread_attr_setstacksize(&attr, stack_size);
    if (rc != 0) {
      handle_errno(rc, "pthread_attr_setstacksize");
    }
  }

  for (int i = 0; i < num_threads; ++i) {
    info[i].thread_num = i + 1;
    info[i].arg = args[i];
    rc = pthread_create(&info[i].thread_id, &attr, &start, &info[i]);
    if (rc != 0) {
      handle_errno(rc, "pthread_create");
    }
  }

  rc = pthread_attr_destroy(&attr);
  if (rc != 0) {
    handle_errno(rc, "pthread_attr_destroy");
  }

  void *res = NULL;
  for (int i = 0; i < num_threads; ++i) {
    rc = pthread_join(info[i].thread_id, &res);
    if (rc != 0) {
      handle_errno(rc, "pthread_join");
    }
    printf("Joined with thread %d; returned value was %s\n",
           info[i].thread_num, (char *)res);
  }
}

int main(int argc, char *argv[]) {
  int opt = -1;
  size_t stack_size = 0;

  while ((opt = getopt(argc, argv, "s:")) != -1) {
    switch (opt) {
    case 's':
      stack_size = strtoul(optarg, NULL, 0);
      break;
    default:
      eprintf("Usage: %s [-s stack-size] arg...\n", argv[0]);
      return EXIT_FAILURE;
    }
  }

  const int num_threads = argc - optind;

  if (num_threads > MAX_THREADS) {
    eprintf("Too many threads; max is %d\n", MAX_THREADS);
    return EXIT_FAILURE;
  }

  run_threads(stack_size, num_threads, &argv[optind]);

  return EXIT_SUCCESS;
}
