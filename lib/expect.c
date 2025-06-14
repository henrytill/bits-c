#include "expect.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_LENGTH 4096

int expect_getpromote(void) {
  char *s = getenv(EXPECT_PROMOTE_VAR);
  if (s == NULL) {
    return 0;
  }
  char *end = NULL;
  long const p = strtol(s, &end, 10);
  if (s == end) {
    return 0;
  }
  return p > 0L;
}

static int fcopy(char const *srcname, char const *dstname) {
  FILE *src = fopen(srcname, "rb");
  if (src == NULL) {
    perror("Error opening source file");
    return -1;
  }

  FILE *dst = fopen(dstname, "wb");
  if (dst == NULL) {
    perror("Error opening destination file");
    (void)fclose(src);
    return -1;
  }

  {
    __label__ cleanup;
    int ret = -1;
    char buf[BUFFER_LENGTH] = {0};
    size_t nread = 0;

    while ((nread = fread(buf, 1, BUFFER_LENGTH, src)) > 0) {
      if (fwrite(buf, 1, nread, dst) != nread) {
        perror("Error writing to destination file");
        goto cleanup;
      }
    }

    if (ferror(src) != 0) {
      perror("Error reading from source file");
      goto cleanup;
    }

    ret = 0;
  cleanup:
    (void)fclose(src);
    (void)fclose(dst);
    return ret;
  }
}

int expect_insert(char const *filename, int lineno, char const *toinsert, int promote) {
  char tempfilename[] = "tempXXXXXX";

  {
    int tempfd = mkstemp(tempfilename);
    if (tempfd == -1) {
      perror("Error creating temporary file");
      return -1;
    }

    FILE *input = fopen(filename, "r");
    if (input == NULL) {
      perror("Error opening input file");
      close(tempfd);
      return -1;
    }

    FILE *output = fdopen(tempfd, "w");
    if (output == NULL) {
      perror("Error opening temporary file");
      close(tempfd);
      (void)fclose(input);
      unlink(tempfilename);
      return -1;
    }

    {
      char *line = NULL;
      size_t len = 0;
      ssize_t nchar;

      for (int i = 1; (nchar = getline(&line, &len, input)) != -1; ++i) {
        if (i == lineno) {
          char *open = strchr(line, '{');
          char *close = strrchr(line, '}');
          if (open != NULL && close != NULL && open < close) {
            (void)fwrite(line, 1, (size_t)(open - line + 1), output);
            (void)fprintf(output, "\"%s\"", toinsert);
            (void)fputs(close, output);
          } else {
            (void)fputs(line, output);
            (void)fprintf(stderr, "Warning: No {} found on line %d\n", lineno);
          }
        } else {
          (void)fputs(line, output);
        }
      }

      free(line);
    }

    (void)fclose(input);
    (void)fclose(output);
  }

  {
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      char *argv[] = {"diff", "-u", (char *)filename, tempfilename, NULL};
      return execv(EXPECT_DIFF_PROGRAM, argv);
    } else {
      int rc = -1;
      wait(NULL);
      if (promote) {
        printf("Promoting %s to %s\n", tempfilename, filename);
        rc = fcopy(tempfilename, filename);
        if (rc != 0) {
          perror("Error copying temporary file to original");
          unlink(tempfilename);
          return -1;
        }
      }
      rc = unlink(tempfilename);
      if (rc != 0) {
        perror("Warning: Could not delete temporary file");
      }
      return 0;
    }
  }
}
