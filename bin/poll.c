#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "macro.h"

static inline void handle_error(char const *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static nfds_t handle_events(nfds_t nfds_open, nfds_t nfds, struct pollfd *pfds) {
  char buf[10];
  ssize_t num_bytes = 0;

  for (nfds_t i = 0; i < nfds; ++i) {
    if (pfds[i].revents == 0) {
      continue;
    }

    printf("fd=%d; events: %s%s%s\n", pfds[i].fd,
           (pfds[i].revents & POLLIN) ? "POLLIN " : "",
           (pfds[i].revents & POLLHUP) ? "POLLHUP " : "",
           (pfds[i].revents & POLLERR) ? "POLLERR " : "");

    if (pfds[i].revents & POLLHUP || pfds[i].revents & POLLERR) {
      printf("closing fd %d\n", pfds[i].fd);
      if (close(pfds[i].fd) == -1) {
        handle_error("close");
      }
      nfds_open -= 1;
      continue;
    }

    num_bytes = read(pfds[i].fd, buf, sizeof(buf));
    if (num_bytes == -1) {
      handle_error("read");
    }
    printf("read %zd bytes: %.*s\n", num_bytes, (int)num_bytes, buf);
  }

  return nfds_open;
}

int main(int argc, char *argv[]) {
  int ready = -1;
  nfds_t nfds_open = 0;
  nfds_t nfds = 0;
  struct pollfd *pfds = NULL;

  if (argc < 2) {
    eprintf("Usage: %s file...\n", argv[0]);
    return EXIT_FAILURE;
  }

  nfds_open = nfds = (nfds_t)argc - 1;

  pfds = calloc(nfds, sizeof(*pfds));
  if (pfds == NULL) {
    handle_error("calloc");
  }

  for (nfds_t i = 0; i < nfds; ++i) {
    pfds[i].fd = open(argv[i + 1], O_RDONLY);
    if (pfds[i].fd == -1) {
      free(pfds);
      handle_error("open");
    }
    printf("Opened \"%s\" on fd %d\n", argv[i + 1], pfds[i].fd);
    pfds[i].events = POLLIN;
  }

  while (nfds_open > 0) {
    printf("About to poll()\n");
    ready = poll(pfds, nfds, -1);
    if (ready == -1) {
      free(pfds);
      handle_error("poll");
    }
    printf("Ready: %d\n", ready);
    nfds_open = handle_events(nfds_open, nfds, pfds);
  }

  printf("All file descriptors closed; bye\n");

  free(pfds);
  return EXIT_SUCCESS;
}
