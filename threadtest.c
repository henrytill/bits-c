#define _POSIX_C_SOURCE 200809L // NOLINT

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "prelude.h"

typedef struct thread_info { // Used as argument to thread_start()
	pthread_t thread_id; // ID returned by pthread_create()
	int thread_num;      // Application-defined thread #
	char *arg;           // From command-line argument
} thread_info;

static inline void handle_errno(int err, const char *msg)
{
	errno = err;
	perror(msg);
	exit(EXIT_FAILURE);
}

///
/// Thread start function
///
/// Display address near top of our stack and return upper-cased copy of argv_string.
///
static void *start(void *arg)
{
	thread_info *info = arg;
	(void)printf("Thread %d: top of stack near %p; argv_string=%s\n",
		info->thread_num, (void *)&info, info->arg);
	char *ret = strdup(info->arg);
	if (ret == NULL) {
		perror("strdup");
		return NULL;
	}
	for (char *p = ret; *p != '\0'; ++p) {
		*p = (char)toupper(*p);
	}
	return ret;
}

int main(int argc, char *argv[])
{
	// The "-s" option specifies a stack size for our threads.
	int opt = -1;
	size_t stack_size = 0;
	while ((opt = getopt(argc, argv, "s:")) != -1) {
		switch (opt) {
		case 's':
			stack_size = strtoul(optarg, NULL, 0);
			break;
		default:
			(void)fprintf(stderr, "Usage: %s [-s stack-size] arg...\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	int num_threads = argc - optind;

	// Initialize thread creation attributes.
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

	// Allocate memory for pthread_create() arguments.
	thread_info *info = ecalloc((size_t)num_threads, sizeof(*info));

	// Create one thread for each command-line argument.
	for (int i = 0; i < num_threads; ++i) {
		info[i].thread_num = i + 1;
		info[i].arg = argv[optind + i];

		// The pthread_create() call stores the thread ID into corresponding element of info[].
		rc = pthread_create(&info[i].thread_id, &attr, &start, &info[i]);
		if (rc != 0) {
			handle_errno(rc, "pthread_create");
		}
	}

	// Destroy the thread attributes object, since it is no longer needed.
	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		handle_errno(rc, "pthread_attr_destroy");
	}

	// Now join with each thread, and display its returned value.
	void *res = NULL;
	for (int i = 0; i < num_threads; ++i) {
		rc = pthread_join(info[i].thread_id, &res);
		if (rc != 0) {
			handle_errno(rc, "pthread_join");
		}
		(void)printf("Joined with thread %d; returned value was %s\n",
			info[i].thread_num, (char *)res);
		free(res); // Free memory allocated by thread
	}

	free(info);
	return EXIT_SUCCESS;
}
