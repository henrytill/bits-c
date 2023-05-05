.POSIX:
.SUFFIXES: .c

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -g
LDFLAGS =

BIN =\
	threadtest

.PHONY: all
all: $(BIN)

threadtest: CFLAGS += -pthread
threadtest: threadtest.c

.PHONY: check test
check test: $(BIN)
	./threadtest foo bar baz

.PHONY: clean
clean:
	rm -f $(BIN)

# Local Variables:
# mode: makefile-bsdmake
# End:
