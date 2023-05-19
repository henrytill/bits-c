.POSIX:
.SUFFIXES:

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -Wconversion -Wsign-conversion -g
LDFLAGS =

BIN =\
	threadtest \
	window

.PHONY: all
all: $(BIN)

threadtest: CFLAGS += -pthread
threadtest: threadtest.c

window: LDFLAGS += -lX11 -lGL -lGLU
window: window.c

.SUFFIXES: .c
.c:
	$(CC) $(CFLAGS) $(.ALLSRC) $(LDFLAGS) -o $@

.PHONY: check test
check test: $(BIN)
	./threadtest foo bar baz

.PHONY: clean
clean:
	rm -f $(BIN)

# Local Variables:
# mode: makefile-bsdmake
# End:
